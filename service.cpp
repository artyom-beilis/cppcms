#define CPPCMS_SOURCE
#include "service.h"
#include "service_impl.h"
#include "applications_pool.h"
#include "thread_pool.h"
#include "global_config.h"
#include "cppcms_error.h"
#include "cgi_acceptor.h"
#include "cgi_api.h"
#include "scgi_api.h"
#include "fastcgi_api.h"
#include "locale_pool.h"

#include "asio_config.h"

#ifdef CPPCMS_POSIX
#include <sys/wait.h>
#endif

#include <iostream>

namespace cppcms {


service::service(int argc,char *argv[]) :
	impl_(new impl::service())
{
	impl_->settings_.reset(new cppcms_config());
	impl_->settings_->load(argc,argv);
	int apps=settings().integer("service.applications_pool_size",threads_no()*2);
	impl_->applications_pool_.reset(new cppcms::applications_pool(*this,apps));

}
service::~service()
{
}

int service::threads_no()
{
	return settings().integer("service.worker_threads",5);
}

namespace {
	cppcms::service *the_service;

#if defined(CPPCMS_WIN32)
	void make_socket_pair(boost::asio::ip::tcp::socket &s1,boost::asio::ip::tcp::socket &s2)
	{
		boost::asio::ip::tcp::acceptor acceptor(s1.get_io_service(),
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0));
		boost::asio::ip::tcp::endpoint server_endpoint = acceptor.local_endpoint();
		server_endpoint.address(boost::asio::ip::address_v4::loopback());
		s1.lowest_layer().connect(server_endpoint);
		acceptor.accept(s2.lowest_layer());
	}

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
	void make_socket_pair(boost::asio::local::stream_protocol::socket &s1,boost::asio::local::stream_protocol::socket &s2)
	{
		boost::asio::local::connect_pair(s1,s2);
	}

	void handler(int nothing)
	{
		the_service->shutdown();
	}

#endif
} // anon


void service::setup_exit_handling()
{
	make_socket_pair(impl_->sig_,impl_->breaker_);

	static char c;

	impl_->breaker_.async_read_some(boost::asio::buffer(&c,1),
					boost::bind(&service::stop,this));

	impl_->notification_socket_=impl_->sig_.native();
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

void service::run()
{
	locale_pool();
	start_acceptor();
	if(prefork()) {
		return;
	}
	thread_pool(); // make sure we start it
	impl_->acceptor_->async_accept();

	setup_exit_handling();

	impl_->get_io_service().run();
}

int service::procs_no()
{
	int procs=settings().integer("service.procs",0);
	if(procs < 0)
		procs = 0;
	#ifdef CPPCMS_WIN32
	if(procs > 0)
		throw cppcms_error("Prefork is not supported under Windows");
	#endif
	return procs;
}

#ifdef CPPCMS_WIN32
bool service::prefork()
{
	procs_no();
	return false;
}
#else // UNIX
bool service::prefork()
{
	int procs=settings().integer("service.procs",0);
	if(procs<=0)
		return false;
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
			return false; // chaild
		}
		else {
			pids[i]=pid;
		}
	}

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGTERM);
	sigaddset(&set,SIGINT);
	sigaddset(&set,SIGQUIT);

	sigprocmask(SIG_BLOCK,&set,NULL);

	int sig;
	do {
		sig=0;
		sigwait(&set,&sig);
	}while(sig!=SIGINT && sig!=SIGTERM && sig!=SIGQUIT);

	sigprocmask(SIG_UNBLOCK,&set,NULL);

	for(int i=0;i<procs;i++) {
		::kill(pids[i],SIGTERM);
		int stat;
		::waitpid(pids[i],&stat,0);
	}
	return true;
}

#endif

void service::start_acceptor()
{
	using namespace impl::cgi;
	std::string api=settings().str("service.api");
	std::string ip=settings().str("service.ip","127.0.0.1");
	int port=0;
	std::string socket=settings().str("service.socket","");
	int backlog=settings().integer("service.backlog",threads_no() * 2);

	bool tcp=socket.empty();

	if(tcp && port==0) {
		port=settings().integer("service.port");
	}

	if(tcp) {
		if(api=="scgi")
			impl_->acceptor_ = scgi_api_tcp_socket_factory(*this,ip,port,backlog);
		else if(api=="fastcgi")
			impl_->acceptor_ = fastcgi_api_tcp_socket_factory(*this,ip,port,backlog);
		else
			throw cppcms_error("Unknown service.api: " + api);
	}
	else {
#ifdef CPPCMS_WIN_NATIVE 
		throw cppcms_error("Unix domain sockets are not supported under Windows... (isn't it obvious?)");
#elif defined CPPCMS_CYGWIN
		throw cppcms_error("CppCMS uses native Win32 sockets under cygwin, so Unix sockets are not supported");
#else
		if(api=="scgi")
			if(socket=="stdin")
				impl_->acceptor_ = scgi_api_unix_socket_factory(*this,backlog);
			else
				impl_->acceptor_ = scgi_api_unix_socket_factory(*this,socket,backlog);
		else if(api=="fastcgi")
			if(socket=="stdin")
				impl_->acceptor_ = fastcgi_api_unix_socket_factory(*this,backlog);
			else
				impl_->acceptor_ = fastcgi_api_unix_socket_factory(*this,socket,backlog);
		else
			throw cppcms_error("Unknown service.api: " + api);
#endif
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

cppcms::cppcms_config const &service::settings()
{
	return *impl_->settings_;
}

cppcms::impl::service &service::impl()
{
	return *impl_;
}

void service::stop()
{
	if(impl_->acceptor_.get())
		impl_->acceptor_->stop();
	thread_pool().stop();
	impl_->get_io_service().stop();
}

locale::pool const &service::locale_pool()
{
	if(!impl_->locale_pool_.get()) {
		impl_->locale_pool_.reset(new locale::pool(settings()));
		
	}
	return *impl_->locale_pool_;
}

namespace impl {
	service::service() :
		io_service_(),
		sig_(io_service_),
		breaker_(io_service_)
	{
	}
	service::~service()
	{
	}
} // impl


} // cppcms
