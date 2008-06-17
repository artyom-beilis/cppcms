#include "manager.h"
#include "cgicc_connection.h"
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include "scgi.h"

namespace cppcms {
namespace details {

fast_cgi_application *fast_cgi_application::handlers_owner=NULL;

fast_cgi_application::fast_cgi_application(cgi_api &a) : api(a)
{
}

fast_cgi_application::event_t fast_cgi_application::wait()
{
	while(!the_end) {
		struct pollfd fds;

		fds.fd=api.get_socket();
		fds.events=POLLIN | POLLERR;
		fds.revents=0;

		/* Wait for two events:
		 * 1. New connection and then do accept
		 * 2. Exit message - exit and do clean up */

		if(poll(&fds,1,-1)<0) {
			if(errno==EINTR && !the_end)
				continue;
			return EXIT;
		}
		else if(fds.revents) {
			if(fds.revents & POLLERR)
				return EXIT;
			return ACCEPT;
		}
	}
	return EXIT;
}

void fast_cgi_application::shutdown()
{
	// Rise exit signal on
	// the selfpipe
	the_end=true;
}

void fast_cgi_application::handler(int id)
{
	fast_cgi_application *ptr=fast_cgi_application::get_instance();
	if(ptr) {
		ptr->shutdown();
	}
}

void fast_cgi_application::set_signal_handlers()
{
	handlers_owner=this;
	/* Signals defined by standard */
	signal(SIGTERM,handler);
	signal(SIGUSR1,handler);
	/* Additional signal */
	signal(SIGINT,handler);
}

fast_cgi_single_threaded_app::fast_cgi_single_threaded_app(base_factory const &factory,cgi_api &a) :
	fast_cgi_application(a)
{
	worker=factory();
}

bool fast_cgi_single_threaded_app::run()
{
	// Blocking loop
	event_t event=wait();

	if(event==EXIT) {
		return false;
	} // ELSE event==ACCEPT

	cgi_session *session=api.accept_session();
	if(session) {
		try {
			if(session->prepare()) {
				worker->run(session->get_connection());
			}
		}
		catch(...) {
			delete session;
			throw;
		}
		delete session;
	}
	return true;
};


void fast_cgi_application::execute()
{
	the_end=false;

	set_signal_handlers();

	while(run()){
		/* Do Continue */
	}
}

fast_cgi_multiple_threaded_app::fast_cgi_multiple_threaded_app(	int threads_num,
								int buffer,
								base_factory const &factory,
								cgi_api &a) :
	fast_cgi_application(a)
{
	int i;

	threads_info=NULL;
	pids=NULL;

	size=threads_num;

	// Init Worker Threads
	workers.resize(size);

	for(i=0;i<size;i++) {
		workers[i]=factory();
	}

	// Setup Jobs Manager
	jobs.init(buffer);

	start_threads();
}

void *fast_cgi_multiple_threaded_app::thread_func(void *p)
{
	info_t *params=(info_t *)p;

	int id=params->first;
	fast_cgi_multiple_threaded_app *self=params->second;

	cgi_session *session;

	while((session=self->jobs.pop())!=NULL) {
		try{
			if(session->prepare()){
				self->workers[id]->run(session->get_connection());
			}
		}
		catch(...){
			delete session;
			return NULL;
		}
		delete session;
	}
	return NULL;
}

void fast_cgi_multiple_threaded_app::start_threads()
{
	int i;

	pids = new pthread_t [size];
	threads_info = new info_t [size];

	for(i=0;i<size;i++) {

		threads_info[i].first=i;
		threads_info[i].second=this;

		pthread_create(pids+i,NULL,thread_func,threads_info+i);
	}
}

void fast_cgi_multiple_threaded_app::wait_threads()
{
	int i;
	for(i=0;i<size;i++) {
		pthread_join(pids[i],NULL);
	}
}

bool fast_cgi_multiple_threaded_app::run()
{
	if(wait()==ACCEPT) {
		cgi_session *session=api.accept_session();
		if(session){
			jobs.push(session);
		}
		return true;
	}
	else {// Exit event
		int i;
		for(i=0;i<size;i++) {
			jobs.push(NULL);
		}

		wait_threads();

		return false;
	}
}
} // END oF Details

void run_application(int argc,char *argv[],base_factory const &factory)
{
	int n,max;
	global_config.load(argc,argv);

	string api=global_config.sval("server.api") ;
	if(api=="cgi") {
		shared_ptr<worker_thread> ptr=factory();
		cgicc_connection_cgi cgi;
		ptr->run(cgi);
	}
	else
	{
		auto_ptr<cgi_api> capi;
		if(api=="fastcgi" || api=="scgi" ) {
			string socket=global_config.sval("server.socket","");
			int backlog=global_config.lval("server.buffer",1);
			if(api=="fastcgi")
				capi=auto_ptr<cgi_api>(new fcgi_api(socket.c_str(),backlog));
			else
				capi=auto_ptr<cgi_api>(new scgi_api(socket.c_str(),backlog));
		}
		else {
			throw cppcms_error("Unknown api:"+api);
		}

		string mod=global_config.sval("server.mod");
		if(mod=="process") {
			details::fast_cgi_single_threaded_app app(factory,*capi.get());
			app.execute();
		}
		else if(mod=="thread") {
			n=global_config.lval("server.threads",5);
			max=global_config.lval("server.buffer",1);
			details::fast_cgi_multiple_threaded_app app(n,max,factory,*capi.get());
			app.execute();
		}
		else {
			throw cppcms_error("Unknown mod:" + mod);
		}
	}
};




} // END OF CPPCMS
