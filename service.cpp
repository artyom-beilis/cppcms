///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "defs.h"
#ifdef CPPCMS_WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <errno.h>
#endif

#ifndef CPPCMS_WIN32
#if defined(__sun)
#define _POSIX_PTHREAD_SEMANTICS
#endif
#include <signal.h>
#endif
#include "service.h"
#include "service_impl.h"
#include "applications_pool.h"
#include "thread_pool.h"
#include "cppcms_error.h"
#include "cgi_acceptor.h"
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

#include "cache_pool.h"
#include "internal_file_server.h"
#include "json.h"
#include "localization.h"
#include "views_pool.h"
#include "session_pool.h"


#ifdef CPPCMS_POSIX
#include <sys/wait.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/lexical_cast.hpp>
#   include <boost/regex.hpp>
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/lexical_cast.hpp>
#   include <cppcms_boost/regex.hpp>
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif


#include <booster/aio/io_service.h>
#include <booster/aio/socket.h>
#include <booster/aio/buffer.h>

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
	impl_->settings_.reset(new json::value());
	load_settings(argc,argv);
	setup();
}

void service::load_settings(int argc,char *argv[])
{
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
	
	json::value &val=*impl_->settings_;


	if(!file_name.empty()) {
		std::ifstream fin(file_name.c_str());
		if(!fin)
			throw cppcms_error("Failed to open filename:"+file_name);
		int line_no=0;
		if(!val.load(fin,true,&line_no)) {
			std::ostringstream ss;
			ss<<"Error reading configurarion file "<<file_name<<" in line:"<<line_no;
			throw cppcms_error(ss.str());
		}
	}
	
	boost::regex r("^--((\\w+)(-\\w+)*)=((true)|(false)|(null)|(-?\\d+(\\.\\d+)?([eE][\\+-]?\\d+)?)|([\\[\\{].*[\\]\\}])|(.*))$");

	for(int i=1;i<argc;i++) {
		std::string arg=argv[i];
		if(arg=="-c") {
			i++;
			continue;
		}
		if(arg=="-U")
			break;
		if(arg.substr(0,2)=="--") {
			boost::cmatch m;
			if(!boost::regex_match(arg.c_str(),m,r))
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
				val.set(path,boost::lexical_cast<double>(m[8].str()));
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

}

void service::setup()
{
	impl_->id_=0;
	int apps=settings().get("service.applications_pool_size",threads_no()*2);
	impl_->applications_pool_.reset(new cppcms::applications_pool(*this,apps));
	impl_->views_pool_.reset(new cppcms::views_pool(settings()));
	impl_->cache_pool_.reset(new cppcms::cache_pool(settings()));
	impl_->session_pool_.reset(new cppcms::session_pool(*this));
}

cppcms::session_pool &service::session_pool()
{
	return *impl_->session_pool_;
}

cppcms::cache_pool &service::cache_pool()
{
	return *impl_->cache_pool_;
}

cppcms::views_pool &service::views_pool()
{
	return *impl_->views_pool_;
}


service::~service()
{
}

int service::threads_no()
{
	return settings().get("service.worker_threads",5);
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
			the_service->shutdown();
			return TRUE;
		default:
			return FALSE;
		}
	}

#else

	void handler(int nothing)
	{
		the_service->shutdown();
	}

#endif
} // anon


void service::setup_exit_handling()
{
	io::socket_pair(io::sock_stream,*impl_->sig_,*impl_->breaker_);

	static char c;

	impl_->breaker_->async_read_some(io::buffer(&c,1),
					boost::bind(&service::stop,this));

	impl_->notification_socket_=impl_->sig_->native();

	if(settings().get("service.disable_global_exit_handling",false))
		return;

	the_service=this;

	#ifdef CPPCMS_WIN32
	SetConsoleCtrlHandler(handler, TRUE);
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

void service::after_fork(function<void()> const &cb)
{
	impl_->on_fork_.push_back(cb);
}

void service::run()
{
	generator();
	session_pool().init();
	start_acceptor();

	if(settings().get("file_server.enable",false))
		applications_pool().mount(applications_factory<cppcms::impl::file_server>(),"");

	if(prefork()) {
		return;
	}
	thread_pool(); // make sure we start it
	
	for(unsigned i=0;i<impl_->on_fork_.size();i++)
		impl_->on_fork_[i]();

	impl_->on_fork_.clear();

	for(unsigned i=0;i<impl_->acceptors_.size();i++)
		impl_->acceptors_[i]->async_accept();

	setup_exit_handling();

	impl_->get_io_service().run();
}

int service::procs_no()
{
	int procs=settings().get("service.worker_processes",0);
	if(procs < 0)
		procs = 0;
	#ifdef CPPCMS_WIN32
	if(procs > 0)
		throw cppcms_error("Prefork is not supported under Windows");
	#else 
	if(procs > 1)
		throw cppcms_error("Prefork support is disabled meanwhile in CppCMS. To Be Fixed");
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
static void  dummy(int n) {}
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
							std::cerr<<"Chaild exited with "<<WEXITSTATUS(status)<<std::endl;
						}
						else if(WIFSIGNALED(status)) {
							std::cerr<<"Chaild killed by "<<WTERMSIG(status)<<std::endl;
						}
						else {
							std::cerr<<"Chaild exited for unknown reason"<<std::endl;
						}
						impl_->id_ = p - pids.begin() + 1;
						*p=-1;
						pid = ::fork();
						if(pid < 0) {
							int err=errno;
							std::cerr<<"Failed to create process: " <<strerror(err)<<std::endl;
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

	for(int i=0;i<procs;i++) {
		if(pids[i]<0)
			continue;
		::kill(pids[i],SIGTERM);
		int stat;
		::waitpid(pids[i],&stat,0);
	}
	return true;
}

#endif

int service::process_id()
{
	return impl_->id_;
}

void service::setup_acceptor(json::value const &v,int backlog)
{
	using namespace cppcms::impl::cgi;

	std::string api=v.get<std::string>("api");
	std::string socket=v.get("socket","");
	std::string ip;
	int port=0;

	bool tcp;
	std::auto_ptr<acceptor> a;

	if(socket.empty()) {
		ip=v.get("ip","127.0.0.1");
		port=v.get("port",8080);
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

	impl_->acceptors_.push_back(boost::shared_ptr<acceptor>(a));
}

void service::start_acceptor()
{
	using namespace impl::cgi;
	int backlog=settings().get("service.backlog",threads_no() * std::max(procs_no(),1) * 2);

	if(	settings().find("service.list").type()!=json::is_undefined 
		&& settings().find("service.api").type()!=json::is_undefined) 
	{
		throw cppcms_error("Can't specify both service.api and service.list");
	}

	if(settings().find("service.api").type()!=json::is_undefined) {
		setup_acceptor(settings()["service"],backlog);
	}
	if(settings().find("service.list").type()!=json::is_undefined) {
		json::array list=settings()["service"].array();
		if(list.empty())
			throw cppcms_error("At least one service should be provided in service.list");
		for(unsigned i=0;i<list.size();i++)
			setup_acceptor(list[i],backlog);
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

void service::post(function<void()> const &handler)
{
	impl_->get_io_service().post(handler);
}

void service::stop()
{
	for(unsigned i=0;i<impl_->acceptors_.size();i++) {
		if(impl_->acceptors_[i])
			impl_->acceptors_[i]->stop();
	}
	thread_pool().stop();
	impl_->get_io_service().stop();
}

locale::generator const &service::generator()
{
	if(impl_->locale_generator_.get())
		return *impl_->locale_generator_.get();

	typedef std::vector<std::string> vstr_type;
	impl_->locale_generator_.reset(new locale::generator());
	locale::generator &gen= *impl_->locale_generator_;
	#ifndef CPPCMS_USE_STD_LOCALES
	gen.characters(locale::char_facet);
	#endif
	std::string enc = settings().get("localization.encoding","");

	if(!enc.empty())
		gen.octet_encoding(enc);

	vstr_type paths= settings().get("localization.messages.paths",vstr_type());
	vstr_type domains = settings().get("localization.messages.domains",vstr_type());

	if(!paths.empty() && !domains.empty()) {
		unsigned i;
		for(i=0;i<paths.size();i++)
			gen.add_messages_path(paths[i]);
		for(i=0;i<domains.size();i++)
			gen.add_messages_domain(domains[i]);
	}

	vstr_type locales = settings().get("localization.locales",vstr_type());

	if(locales.empty()) {
		gen.preload("");
		impl_->default_locale_=gen("");
	}
	else {
		for(unsigned i=0;i<locales.size();i++)
			gen.preload(locales[i]);
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
	return generator().get(name);
}

booster::aio::io_service &service::get_io_service()
{
	return impl_->get_io_service();
}


namespace impl {
	service::service() :
		io_service_(new io::io_service()),
		sig_(new io::socket(*io_service_)),
		breaker_(new io::socket(*io_service_))
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


} // cppcms
