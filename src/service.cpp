///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/defs.h>
#ifdef CPPCMS_WIN32
#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#include <winsvc.h>
#include <process.h>
#else
#include <errno.h>
#endif

#ifndef CPPCMS_WIN32
#if defined(__sun)
#define _POSIX_PTHREAD_SEMANTICS
#endif
#include <signal.h>
#endif
#include <cppcms/service.h>
#include "service_impl.h"
#include <cppcms/applications_pool.h>
#include <cppcms/application.h>
#include <cppcms/thread_pool.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/mount_point.h>
#include <cppcms/forwarder.h>
#include <cppcms/mem_bind.h>
#include <cppcms/plugin.h>
#include "cgi_acceptor.h"
#ifndef CPPCMS_WIN32
#include "prefork_acceptor.h"
#endif
#include "cgi_api.h"
#ifdef CPPCMS_HAS_SCGI
# include "scgi_api.h"
#endif
#ifdef CPPCMS_HAS_HTTP
# include "http_api.h"
#endif
#ifdef CPPCMS_HAS_FCGI
# include "fastcgi_api.h"
#endif

#ifdef CPPCMS_WIN_NATIVE
#include "winservice.h"
#endif

#include "cached_settings.h"
#include "logging.h"
#include <cppcms/cache_pool.h>
#include "internal_file_server.h"
#include <cppcms/json.h>
#include <cppcms/localization.h>
#include <cppcms/views_pool.h>
#include <cppcms/session_pool.h>

#include <booster/log.h>

#ifdef CPPCMS_POSIX
#include <sys/wait.h>
#include <syslog.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <utility>
#include <booster/nowide/fstream.h>
#include <booster/thread.h>
#include <cppcms/config.h>

#ifndef CPPCMS_WIN32
#include "daemonize.h"
#endif

#include <booster/regex.h>




#include <booster/aio/io_service.h>
#include <booster/aio/socket.h>
#include <booster/aio/buffer.h>
#include <booster/aio/reactor.h>

namespace cppcms {

service::service(json::value const &v) :
	impl_(new impl::service())
{
	impl_->settings_.reset(new json::value(v));
	setup();
}

service::service(int argc,char *argv[]) :
	impl_(new impl::service())
{
	impl_->args_.assign(argv,argv+argc);
	json::value val = load_settings(argc,argv);
	impl_->settings_.reset(new json::value());
	impl_->settings_->swap(val);
	setup();
}

json::value service::load_settings(int argc,char *argv[])
{
	json::value val;
	using std::string;
	std::string file_name;
	
	for(int i=1;i<argc;i++) {
		if(argv[i]==std::string("-c")) {
			if(!file_name.empty()) {
				throw cppcms_error("Switch -c can't be used twice");
			}
			if(i+1 < argc)
				file_name=argv[i+1];
			else
				throw cppcms_error("Switch -c requires configuration file parameters");
			i++;
		}
		else if(argv[i]==std::string("-U")) 
			break;
	}
	
	if(file_name.empty()) {
		char const *e = getenv("CPPCMS_CONFIG");
		if(e) file_name = e;
	}
	
	if(!file_name.empty()) {
		booster::nowide::ifstream fin(file_name.c_str());
		if(!fin)
			throw cppcms_error("Failed to open filename:"+file_name);
		int line_no=0;
		if(!val.load(fin,true,&line_no)) {
			std::ostringstream ss;
			ss<<"Error reading configuration file "<<file_name<<" in line:"<<line_no;
			throw cppcms_error(ss.str());
		}
	}
	
	booster::regex r("^--((\\w+)(-\\w+)*)=((true)|(false)|(null)|(-?\\d+(\\.\\d+)?([eE][\\+-]?\\d+)?)|([\\[\\{].*[\\]\\}])|(.*))$");

	for(int i=1;i<argc;i++) {
		std::string arg=argv[i];
		if(arg=="-c") {
			i++;
			continue;
		}
		if(arg=="-U")
			break;
		if(arg.substr(0,2)=="--") {
			booster::cmatch m;
			if(!booster::regex_match(arg.c_str(),m,r))
				throw cppcms_error("Invalid switch: "+arg);
			std::string path=m[1];
			for(unsigned i=0;i<path.size();i++)
				if(path[i]=='-')
					path[i]='.';
			if(!m[5].str().empty())
				val.set(path,true);
			else if(!m[6].str().empty())
				val.set(path,false);
			else if(!m[7].str().empty())
				val.set(path,json::null());
			else if(!m[8].str().empty())
				val.set(path,atof(m[8].str().c_str()));
			else if(!m[11].str().empty()) {
				std::istringstream ss(m[11].str());
				json::value tmp;
				if(!tmp.load(ss,true))
					throw cppcms_error("Invadid json expresson: " + m[11].str());
				val.set(path,tmp);
			}
			else
				val.set(path,m[4].str());
		}
	}

	if(val.is_undefined()) {
		throw cppcms_error("No configuration defined");
	}
	return val;

}

namespace {
	int reactor_type(std::string const &name)
	{
		using booster::aio::reactor;
		if(name=="select")
			return reactor::use_select;
		if(name=="poll")
			return reactor::use_poll;
		if(name=="epoll")
			return reactor::use_epoll;
		if(name=="devpoll")
			return reactor::use_dev_poll;
		if(name=="kqueue")
			return reactor::use_kqueue;
		return reactor::use_default;
	}
}


impl::cached_settings const &service::cached_settings()
{
	return *impl_->cached_settings_;
}

void service::setup()
{
	impl_->cached_settings_.reset(new impl::cached_settings(settings()));
	impl::setup_logging(settings());
	impl_->id_=0;
	int reactor=reactor_type(settings().get("service.reactor","default"));
	impl_->io_service_.reset(new io::io_service(reactor));
	impl_->sig_.reset(new io::stream_socket(*impl_->io_service_));
	impl_->breaker_.reset(new io::stream_socket(*impl_->io_service_));

	impl_->plugins_.reset(new cppcms::plugin::scope(settings()));
	impl_->applications_pool_.reset(new cppcms::applications_pool(*this,0));
	impl_->views_pool_.reset(new cppcms::views::manager(settings()));
	impl_->cache_pool_.reset(new cppcms::cache_pool(settings()));
	impl_->session_pool_.reset(new cppcms::session_pool(*this));
	if(settings().get("file_server.enable",false)) {
		int flags = app::synchronous;
		if(settings().get("file_server.async",false)) {
			flags = app::asynchronous;
		}
		applications_pool().mount(create_pool<cppcms::impl::file_server>(),mount_point(""),flags);
	}
}

void impl::setup_logging(json::value const &settings)
{
	using namespace booster::log;
	level_type level = logger::string_to_level(settings.get("logging.level","error"));
	logger::instance().set_default_level(level);
	if(	(
			settings.find("logging.file").is_undefined()
			&& settings.find("logging.syslog").is_undefined()
			&& settings.find("logging.stderr").is_undefined()
		)
		|| 
		settings.get("logging.stderr",false)==true
	  )
	{
		logger::instance().add_sink(booster::shared_ptr<sink>(new sinks::standard_error()));
	}
	if(settings.get("logging.syslog.enable",false)==true) {
		#ifndef CPPCMS_POSIX
			throw cppcms_error("Syslog is not availible on Windows");
		#else
		std::string id = settings.get("logging.syslog.id","");
		std::string facility = settings.get("logging.syslog.facility","user");
		std::vector<std::string> vops = settings.get("logging.syslog.options",std::vector<std::string>());
		std::string sfacility = settings.get("logging.syslog.options","");
		int facility_code = 0;
		int ops = 0;
		for(unsigned i=0;i<vops.size();i++) {
			std::string const &op=vops[i];
			if(op=="LOG_CONS") ops|=LOG_CONS;
			else if(op=="LOG_NDELAY") ops|=LOG_NDELAY;
			else if(op=="LOG_NOWAIT") ops|=LOG_NOWAIT;
			else if(op=="LOG_ODELAY") ops|=LOG_ODELAY;
			#ifdef LOG_PERROR
			else if(op=="LOG_PERROR") ops|=LOG_PERROR;
			#endif
			else if(op=="LOG_PID") ops|=LOG_PID;
		}
		if(!facility.empty()) {
			if(facility == "user") {
				facility_code = LOG_USER;
			} else if(facility == "local0") {
				facility_code = LOG_LOCAL0;
			} else if(facility == "local1") {
				facility_code = LOG_LOCAL1;
			} else if(facility == "local2") {
				facility_code = LOG_LOCAL2;
			} else if(facility == "local3") {
				facility_code = LOG_LOCAL3;
			} else if(facility == "local4") {
				facility_code = LOG_LOCAL4;
			} else if(facility == "local5") {
				facility_code = LOG_LOCAL5;
			} else if(facility == "local6") {
				facility_code = LOG_LOCAL6;
			} else if(facility == "local7") {
				facility_code = LOG_LOCAL7;
			} else {
				throw cppcms_error("Unsupported syslog facility: " + facility);
			}
		}
		if(id.empty())
			logger::instance().add_sink(booster::shared_ptr<sink>(new sinks::syslog(ops,facility_code)));
		else
			logger::instance().add_sink(booster::shared_ptr<sink>(new sinks::syslog(id,ops,facility_code)));
		#endif
	}

	std::string log_file;
	if(!(log_file=settings.get("logging.file.name","")).empty()) {
		booster::shared_ptr<sinks::file> file(new sinks::file());
		int max_files=0;
		if((max_files = settings.get("logging.file.max_files",0)) > 0)
			file->max_files(max_files);
		bool append = false;
		if((append = settings.get("logging.file.append",false))==true)
			file->append();
		file->open(log_file);
		std::string tz = settings.get("logging.file.timezone","");
		file->set_timezone(tz);
		logger::instance().add_sink(file);
	}
}

cppcms::session_pool &service::session_pool()
{
	return *impl_->session_pool_;
}

cppcms::cache_pool &service::cache_pool()
{
	return *impl_->cache_pool_;
}

cppcms::views::manager &service::views_pool()
{
	return *impl_->views_pool_;
}


service::~service()
{
}

int service::threads_no()
{
	return cached_settings().service.worker_threads;
}

namespace {
	cppcms::service *the_service;

#if defined(CPPCMS_WIN32)

	BOOL WINAPI handler(DWORD ctrl_type)
	{
		switch (ctrl_type)
		{
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			if(the_service)
				the_service->shutdown();
			return TRUE;
		default:
			return FALSE;
		}
	}

	class apache_event_handler {
	public:
		apache_event_handler(HANDLE h=0) : handle_(h)
		{
		}
		void operator()() const
		{
			if( ::WaitForSingleObject(handle_,INFINITE) == WAIT_OBJECT_0 ) {
				if(the_service) {
					the_service->shutdown();
				}
			}
		}
	private:
		HANDLE handle_;
	};

#else

	void handler(int /*nothing*/)
	{
		if(the_service)
			the_service->shutdown();
	}

#endif
} // anon


namespace {
	struct stop_binder {
		stop_binder(void (cppcms::service::*member)(),cppcms::service *s) : m(member),srv(s) {}
		void (cppcms::service::*m)();
		cppcms::service *srv;
		void operator()(booster::system::error_code const &,size_t) 
		{
			(srv->*m)();
		}
	};
}

void service::setup_exit_handling()
{
	io::socket_pair(*impl_->sig_,*impl_->breaker_);

	static char c;

	impl_->breaker_->async_read_some(io::buffer(&c,1),stop_binder(&service::stop,this));

	impl_->notification_socket_=impl_->sig_->native();

	if(settings().get("service.disable_global_exit_handling",false))
		return;

	the_service=this;

	#ifdef CPPCMS_WIN32
	SetConsoleCtrlHandler(handler, TRUE);
	char *event = getenv("_FCGI_SHUTDOWN_EVENT_");
	if(event) {
		apache_event_handler handler((HANDLE)(atoi(event)));
		booster::thread wait_for_event(handler);
	}
	#else
	struct sigaction sa;

	memset(&sa,0,sizeof(sa));
	sa.sa_handler=handler;
	
	sigaction(SIGINT,&sa,0);
	sigaction(SIGTERM,&sa,0);
	sigaction(SIGUSR1,&sa,0);
	#endif
}



void service::shutdown()
{
	char c='A';
#ifdef CPPCMS_WIN32
	if(send(impl_->notification_socket_,&c,1,0) <= 0) {
		perror("notification failed");
		exit(1);
	}
#else
	for(;;){
		int res=::write(impl_->notification_socket_,&c,1);
		if(res<0 && errno == EINTR)
			continue;
		if(res<=0) {
			perror("shudown notification failed");
			exit(1);
		}
		return;
	}
#endif
}

void service::after_fork(booster::function<void()> const &cb)
{
	impl_->on_fork_.push_back(cb);
}

void service::after_fork_exec()
{

	for(unsigned i=0;i<impl_->on_fork_.size();i++)
		impl_->on_fork_[i]();
	impl_->on_fork_.clear();
}


cppcms::forwarder &service::forwarder()
{
	if(!impl_->forwarder_.get()) {
		impl_->forwarder_.reset(new cppcms::forwarder());
		if(settings().find("forwarding.rules").type()==json::is_array) {
			json::array rules=settings().at("forwarding.rules").array();
			for(unsigned i=0;i<rules.size();i++) {
				mount_point mp;
				if(rules[i].find("host").type()==json::is_string) 
					mp.host(booster::regex(rules[i].get<std::string>("host")));
				if(rules[i].find("script_name").type()==json::is_string) 
					mp.script_name(booster::regex(rules[i].get<std::string>("script_name")));
				if(rules[i].find("path_info").type()==json::is_string) 
					mp.path_info(booster::regex(rules[i].get<std::string>("path_info")));
				std::string ip=rules[i].get<std::string>("ip");
				int port=rules[i].get<int>("port");
				impl_->forwarder_->add_forwarding_rule(booster::shared_ptr<mount_point>(new mount_point(mp)),ip,port);
			}
		}
	}
	return *impl_->forwarder_;
}


void service::run_prepare()
{
	generator();
	forwarder();
	session_pool().init();
	start_acceptor();
}

void service::run_acceptor()
{
	for(unsigned i=0;i<impl_->acceptors_.size();i++)
		impl_->acceptors_[i]->async_accept();
}

void service::run_event_loop()
{
	impl_->get_io_service().run();
}


#ifdef CPPCMS_WIN32

void service::run_win_console()
{
	run_prepare();	
	prefork(); // not really prefork
	after_fork_exec();
	thread_pool();
	run_acceptor();
	setup_exit_handling();
	
	run_event_loop();
}

#endif

#ifdef CPPCMS_WIN_NATIVE

void service::win_service_prepare()
{
	run_prepare();	
	prefork(); // not really prefork
	after_fork_exec();
	thread_pool();
	run_acceptor();
	setup_exit_handling();
}

void service::win_service_exec()
{
	run_event_loop();
}

void service::run_win_service()
{
	using cppcms::util::mem_bind;
	impl::winservice::instance().prepare(mem_bind(&service::win_service_prepare,this));
	impl::winservice::instance().exec(mem_bind(&service::win_service_exec,this));
	impl::winservice::instance().stop(mem_bind(&service::shutdown,this));
	
 	int argc=impl_->args_.size();
	std::vector<char*> argv(argc+1,static_cast<char*>(0));
	for(int i=0;i<argc;i++) {
		argv[i]=const_cast<char *>(impl_->args_[i].c_str());
	}
	
  	impl::winservice::instance().run(*impl_->settings_,argc,&argv[0]);
}
#endif

#ifdef CPPCMS_WIN32
void service::run()
{
#ifdef CPPCMS_WIN_NATIVE
	if(settings().get("winservice.mode","console") == "console") 
		run_win_console();
	else
		run_win_service();
#else
	run_win_console();
#endif
}
#else // POSIX
void service::run()
{
	run_prepare();

	impl::daemonizer godaemon(settings());
	
	if(prefork()) {
		return;
	}

	thread_pool(); // make sure we start it

	if(impl_->prefork_acceptor_.get())
		impl_->prefork_acceptor_->start();
	
	after_fork_exec();
	run_acceptor();
	setup_exit_handling();
	
	run_event_loop();
}

#endif

int service::procs_no()
{
	int procs=cached_settings().service.worker_processes;
	if(procs < 0)
		procs = 0;
	#ifdef CPPCMS_WIN32
	if(procs > 0)
		throw cppcms_error("Prefork is not supported under Windows and Cygwin");
	#endif
	return procs;
}

#ifdef CPPCMS_WIN32
bool service::prefork()
{
	procs_no();
	impl_->id_ = 1;
	return false;
}
#else // UNIX
static void  dummy(int /*n*/) {}
bool service::prefork()
{
	sigset_t pipemask;
	sigemptyset(&pipemask);
	sigaddset(&pipemask,SIGPIPE);
	sigprocmask(SIG_BLOCK,&pipemask,0);

	int procs=procs_no();
	if(procs==0) {
		impl_->id_ = 1;
		return false;
	}
	std::vector<int> pids(procs,0);
	
	for(int i=0;i<procs;i++) {
		int pid=::fork();
		if(pid < 0) {
			int err=errno;
			for(int j=0;j<i;j++) {
				::kill(pids[j],SIGTERM);
			}
			for(int j=0;j<i;j++) {
				int stat;
				::waitpid(pids[j],&stat,0);
			}
			throw cppcms_error(err,"fork failed");
		}
		else if(pid==0) {
			impl_->id_ = i+1;
			return false; // chaild
		}
		else {
			pids[i]=pid;
		}
	}

	sigset_t set,prev;
	sigemptyset(&set);
	sigaddset(&set,SIGTERM);
	sigaddset(&set,SIGINT);
	sigaddset(&set,SIGCHLD);
	sigaddset(&set,SIGQUIT);

	sigprocmask(SIG_BLOCK,&set,&prev);

	// Enable delivery of SIGCHLD
	struct sigaction sa_new,sa_old;
	memset(&sa_new,0,sizeof(sa_new));
	sa_new.sa_handler=dummy;
	::sigaction(SIGCHLD,&sa_new,&sa_old);

	int sig;
	do {
		sig=0;
		sigwait(&set,&sig);
		BOOSTER_INFO("cppcms") << "Catched signal " << sig;
		if(sig==SIGCHLD) {
			int status;
			int pid = ::waitpid(0,&status,WNOHANG);
			if(pid > 0)  {
				std::vector<int>::iterator p;
				p = std::find(pids.begin(),pids.end(),pid);
				// Ingnore all processes that are not my own childrens
				if(p!=pids.end()) {
					// TODO: Make better error handling
					if(!WIFEXITED(status) || WEXITSTATUS(status)!=0){
						if(WIFEXITED(status)) {
							BOOSTER_CRITICAL("cppcms") <<"Chaild exited with "<<WEXITSTATUS(status);
						}
						else if(WIFSIGNALED(status)) {
							BOOSTER_CRITICAL("cppcms") <<"Chaild killed by "<<WTERMSIG(status);
						}
						else {
							BOOSTER_CRITICAL("cppcms") <<"Chaild exited for unknown reason";
						}
						impl_->id_ = p - pids.begin() + 1;
						*p=-1;
						pid = ::fork();
						if(pid < 0) {
							int err=errno;
							BOOSTER_ALERT("cppcms") <<"Failed to create process: " <<strerror(err);
						}
						else if(pid == 0) {
							::sigaction(SIGCHLD,&sa_old,NULL);
							sigprocmask(SIG_SETMASK,&prev,NULL);
							return false;
						}
						else {
							*p=pid;
							impl_->id_ = 0;
						}
					}
					else {
						*p=-1;
					}
				}
			}
		}
	}while(sig!=SIGINT && sig!=SIGTERM && sig!=SIGQUIT);

	sigprocmask(SIG_SETMASK,&prev,NULL);
	
	BOOSTER_INFO("cppcms") << "Shutting down";

	BOOSTER_DEBUG("cppcms") << "Killing Children";
	for(int i=0;i<procs;i++) {
		if(pids[i]<0)
			continue;
		::kill(pids[i],SIGTERM);
	}

	for(int i=0;i<procs;i++) {
		if(pids[i]<0)
			continue;
		int stat;
		::waitpid(pids[i],&stat,0);
	}
	BOOSTER_DEBUG("cppcms") << "Children are dead";
	return true;
}

#endif

int service::process_id()
{
	return impl_->id_;
}

std::unique_ptr<cppcms::impl::cgi::acceptor> service::setup_acceptor(json::value const &v,int backlog,int port_shift)
{
	using namespace cppcms::impl::cgi;

	std::string api=v.get<std::string>("api");
	std::string socket=v.get("socket","");
	int sndbuf = v.get("sndbuf",-1);
	int rcvbuf = v.get("rcvbuf",-1);
	std::string ip;
	int port=0;

	bool tcp;
	std::unique_ptr<acceptor> a;

	if(socket.empty()) {
		ip=v.get("ip","127.0.0.1");
		port=v.get("port",8080) + port_shift;
		tcp=true;
	}
	else {
		if(	!v.find("port").is_undefined()
			|| !v.find("ip").is_undefined())
		{
			throw cppcms_error("Can't define both UNIX socket and TCP port/ip");
		}
		tcp=false;
	}

	if(tcp) {
		#ifdef CPPCMS_HAS_SCGI
		if(api=="scgi")
			a = scgi_api_tcp_socket_factory(*this,ip,port,backlog);
		#endif		
		#ifdef CPPCMS_HAS_FCGI
		if(api=="fastcgi")
			a = fastcgi_api_tcp_socket_factory(*this,ip,port,backlog);
		#endif
		#ifdef CPPCMS_HAS_HTTP
		if(api=="http")
			a = http_api_factory(*this,ip,port,backlog);
		#endif
	}
	else {
#ifdef CPPCMS_WIN_NATIVE 
		throw cppcms_error("Unix domain sockets are not supported under Windows... (isn't it obvious?)");
#elif defined CPPCMS_CYGWIN
		throw cppcms_error("CppCMS uses native Win32 sockets under cygwin, so Unix sockets are not supported");
#else
		#ifdef CPPCMS_HAS_SCGI
		if(api=="scgi") {
			if(socket=="stdin")
				a = scgi_api_unix_socket_factory(*this,backlog);
			else
				a = scgi_api_unix_socket_factory(*this,socket,backlog);
		}
		#endif
		
		#ifdef CPPCMS_HAS_FCGI
		if(api=="fastcgi") {
			if(socket=="stdin")
				a = fastcgi_api_unix_socket_factory(*this,backlog);
			else
				a = fastcgi_api_unix_socket_factory(*this,socket,backlog);
		}
		#endif

		#ifdef CPPCMS_HAS_HTTP
		if(api=="http")
			throw cppcms_error("HTTP API is not supported over Unix Domain sockets");
		#endif
#endif
	}
	if(!a.get())
		throw cppcms_error("Unknown api: " + api);

	a->sndbuf(sndbuf);
	a->rcvbuf(rcvbuf);

	return a;
}

void service::start_acceptor(bool after_fork)
{
	using namespace impl::cgi;
	int backlog=settings().get("service.backlog",threads_no() * std::max(procs_no(),1) * 2);
	bool preforking = procs_no() > 1 && !after_fork;
	if(preforking)
	#ifndef CPPCMS_WIN32
		impl_->prefork_acceptor_.reset(new impl::prefork_acceptor(this));
	#else
		throw cppcms_error("Preforking is not supported under Windows");
	#endif

	if(	settings().find("service.list").type()!=json::is_undefined 
		&& settings().find("service.api").type()!=json::is_undefined) 
	{
		throw cppcms_error("Can't specify both service.api and service.list");
	}

	if(settings().find("service.api").type()!=json::is_undefined) {
		booster::shared_ptr<acceptor> ac(setup_acceptor(settings()["service"],backlog));
		#ifndef CPPCMS_WIN32
		if(preforking) {
			impl_->prefork_acceptor_->add_acceptor(ac);
		}
		else 
		#endif
		{
			impl_->acceptors_.push_back(ac);
		}
	}
	if(settings().find("service.list").type()!=json::is_undefined) {
		json::array list=settings()["service"]["list"].array();
		if(list.empty())
			throw cppcms_error("At least one service should be provided in service.list");
		for(unsigned i=0;i<list.size();i++) {
			booster::shared_ptr<acceptor> ac(setup_acceptor(list[i],backlog));
			#ifndef CPPCMS_WIN32
			if(preforking) {
				impl_->prefork_acceptor_->add_acceptor(ac);
			}
			else 
			#endif
			{
				impl_->acceptors_.push_back(ac);
			}
		}
	}
}

cppcms::applications_pool &service::applications_pool()
{
	return *impl_->applications_pool_;
}
cppcms::thread_pool &service::thread_pool()
{
	if(!impl_->thread_pool_.get()) {
		impl_->thread_pool_.reset(new cppcms::thread_pool(threads_no()));
	}
	return *impl_->thread_pool_;
}

json::value const &service::settings()
{
	return *impl_->settings_;
}

cppcms::impl::service &service::impl()
{
	return *impl_;
}

void service::post(booster::function<void()> const &handler)
{
	impl_->get_io_service().post(handler);
}

void service::stop()
{
	for(unsigned i=0;i<impl_->acceptors_.size();i++) {
		if(impl_->acceptors_[i])
			impl_->acceptors_[i]->stop();
	}
	#ifndef CPPCMS_WIN32
	impl_->prefork_acceptor_.reset();
	#endif
	thread_pool().stop();
	impl_->get_io_service().stop();
}

locale::generator const &service::generator()
{
	if(impl_->locale_generator_.get())
		return *impl_->locale_generator_.get();

	typedef std::vector<std::string> vstr_type;
	
	std::string default_backend = settings().get("localization.backend","");
	if(default_backend.empty())
		impl_->locale_generator_.reset(new locale::generator());
	else {
		locale::localization_backend_manager mgr = locale::localization_backend_manager::global();
		mgr.select(default_backend);
		impl_->locale_generator_.reset(new locale::generator(mgr));
	}
	locale::generator &gen= *impl_->locale_generator_;
	gen.characters(locale::char_facet);
	std::string enc = settings().get("localization.encoding","");

	vstr_type paths= settings().get("localization.messages.paths",vstr_type());
	vstr_type domains = settings().get("localization.messages.domains",vstr_type());

	if(!paths.empty() && !domains.empty()) {
		unsigned i;
		for(i=0;i<paths.size();i++) {
			gen.add_messages_path(paths[i]);
		}
		for(i=0;i<domains.size();i++)
			gen.add_messages_domain(domains[i]);
	}

	vstr_type locales = settings().get("localization.locales",vstr_type());
	gen.locale_cache_enabled(true);

	if(locales.empty()) {
		gen("");
		impl_->default_locale_=gen("");
		if(std::use_facet<locale::info>(impl_->default_locale_).name()=="C")
			BOOSTER_WARNING("cppcms") 
				<< "The default system locale is 'C', the encoding is set to US-ASCII. "
				<< "It is recommended to specify the locale name explicitly";
	}
	else {
		for(unsigned i=0;i<locales.size();i++) {
			std::locale tmp = gen(locales[i]);
			locale::info const &inf = std::use_facet<locale::info>(tmp);
			if(std::use_facet<locale::info>(tmp).name()=="C" || inf.encoding()=="us-ascii") {
				if(locales[i].empty()) {
					BOOSTER_WARNING("cppcms") 
						<< "The default system locale is 'C', the encoding is set to US-ASCII. "
						<< "It is recommended to specify the locale name explicitly";
				}
				else if(locales[i].find('.')==std::string::npos) {
					BOOSTER_WARNING("cppcms") 
						<< "The encoding for locale `" << locales[i] << "' is not specified "
						<< "the encoding is set to US-ASCII. It is recommended to specify the locale name explicitly";
				}
			}
		}
		impl_->default_locale_=gen(locales[0]);
	}

	return *impl_->locale_generator_.get();

}

std::locale service::locale()
{
	generator();
	return impl_->default_locale_;
}
std::locale service::locale(std::string const &name)
{
	return generator().generate(name);
}

booster::aio::io_service &service::get_io_service()
{
	return impl_->get_io_service();
}

namespace impl {
	service::service() 
	{
	}
	service::~service()
	{
		acceptors_.clear();
		thread_pool_.reset();
		sig_.reset();
		breaker_.reset();
		io_service_.reset();
		// applications pool should be destroyed after
		// io_service, because soma apps may try unregister themselfs
		applications_pool_.reset();
		locale_generator_.reset();
settings_.reset();
		
	}
} // impl

plugin::scope &service::plugins()
{
	return *impl_->plugins_;
}


} // cppcms
