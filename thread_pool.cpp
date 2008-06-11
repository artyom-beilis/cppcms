#include "thread_pool.h"

#include <poll.h>
#include <signal.h>
#include <errno.h>

namespace cppcms {
namespace details {

fast_cgi_application *fast_cgi_application::handlers_owner=NULL;

fast_cgi_application::fast_cgi_application(const char *socket,int backlog)
{
	FCGX_Init();
	
	pipe(signal_pipe);
	if(socket==NULL || socket[0]==0){
		main_fd=0;
	}
	else {
		this->socket=socket;
		main_fd=FCGX_OpenSocket(socket,backlog);
		if(main_fd<0) {
			throw cppcms_error(string("Failed to open socket ")
					+socket);
		}
	}
}

fast_cgi_application::event_t fast_cgi_application::wait()
{
	for(;;) {
		struct pollfd fds[2];
			
		fds[0].fd=main_fd;
		fds[0].events=POLLIN | POLLERR;
		fds[0].revents=0;
		
		fds[1].fd=signal_pipe[0];
		fds[1].events=POLLIN | POLLERR;
		fds[1].revents=0;

		/* Wait for two events:
		 * 1. New connection and then do accept
		 * 2. Exit message - exit and do clean up */
		
		if(poll(fds,2,-1)<0) {
			if(errno==EINTR)
				continue;
			return EXIT;
		}
		
		if(fds[1].revents) {
			return EXIT;
		}
		else if(fds[0].revents) {
			if(fds[0].revents & POLLERR)
				return EXIT;
			return ACCEPT;
		}
	}
}

void fast_cgi_application::shutdown()
{
	// Rise exit signal on
	// the selfpipe
	write(signal_pipe[1],"0",1);
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

fast_cgi_single_threaded_app::fast_cgi_single_threaded_app(base_factory const &factory,char const *socket) :
	fast_cgi_application(socket,1)
{
	worker=factory();
	FCGX_InitRequest(&request, main_fd, 0);
}

bool fast_cgi_single_threaded_app::run()
{
	// Blocking loop
	event_t event=wait();
	
	if(event==EXIT) {
		return false;
	} // ELSE event==ACCEPT
	
	int res;
	
	res=FCGX_Accept_r(&request);
	
	if(res>=0){
		// Execute 
		worker->run(&request);
	}
	return true;
};


void fast_cgi_application::execute()
{
	set_signal_handlers();
	
	while(run()){
		/* Do Continue */
	}
}

fast_cgi_multiple_threaded_app::fast_cgi_multiple_threaded_app(	int threads_num,
								int buffer,
								base_factory const &factory,
								char const *socket) :
	fast_cgi_application(socket,threads_num+1+buffer)
{
	int i;
	
	requests=NULL;
	threads_info=NULL;
	pids=NULL;
	
	size=threads_num;

	// Init Worker Threads
	workers.resize(size);

	for(i=0;i<size;i++) {
		workers[i]=factory();
	}
	
	// Init FCGX Requests
	requests = new FCGX_Request [buffer];
	for(i=0;i<buffer;i++) {
		FCGX_InitRequest(requests+i, main_fd, 0);
	}
	
	requests_queue.init(buffer);
	for(i=0;i<buffer;i++)
		requests_queue.push(requests+i);
	
	// Setup Jobs Manager
	jobs_queue.init(buffer);

	start_threads();
}

void *fast_cgi_multiple_threaded_app::thread_func(void *p)
{
	info_t *params=(info_t *)p;

	int id=params->first;
	fast_cgi_multiple_threaded_app *self=params->second;

	FCGX_Request *req;
	
	while((req=self->jobs_queue.pop())!=NULL) {
		self->workers[id]->run(req);
		self->requests_queue.push(req);
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
	int res;
	if(wait()==ACCEPT) {
		FCGX_Request *req=requests_queue.pop();
		
		res=FCGX_Accept_r(req);
		
		if(res<0) {
			requests_queue.push(req);
			return true;
		}

		jobs_queue.push(req);
		return true;
	}
	else {// Exit event 
		int i;
		for(i=0;i<size;i++) {
			jobs_queue.push(NULL);
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
	
	char const *socket=global_config.sval("server.socket","").c_str();
	
	if((n=global_config.lval("server.threads",0))==0) {
		details::fast_cgi_single_threaded_app app(factory,socket);
		app.execute();
	}
	else {
		max=global_config.lval("server.buffer",1);
		details::fast_cgi_multiple_threaded_app app(n,max,factory,socket);
		app.execute();
	}
};



} // END OF CPPCMS
