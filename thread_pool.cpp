#include "thread_pool.h"

#include <poll.h>
#include <signal.h>

using namespace cppcms;
using namespace cppcms::details;

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
		fds[0].events=POLLIN;
		fds[0].revents=0;
		
		fds[1].fd=signal_pipe[0];
		fds[1].events=POLLIN;
		fds[1].revents=0;

		/* Wait for two events:
		 * 1. New connection and then do accept
		 * 2. Exit message - exit and do clean up */
		
		poll(fds,2,-1);
		
		if(fds[1].revents) {
			return EXIT;
		}
		else if(fds[0].revents) {
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
void fast_cgi_single_threaded_app::setup(worker_thread *worker)
{
	this->worker=worker;
	worker->init();
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
	
	#ifdef FCGX_API_ACCEPT_ONLY_EXISTS
	res=FCGX_Accept_Only_r(&request);
	#else 
	res=FCGX_Accept_r(&request);
	#endif
	
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

void *fast_cgi_multiple_threaded_app::thread_func(void *p)
{
	info_t *params=(info_t *)p;

	int id=params->first;
	fast_cgi_multiple_threaded_app *me=params->second;

	FCGX_Request *req;
	
	while((req=me->jobs_queue.pop())!=NULL) {
		me->workers[id]->run(req);
		me->requests_queue.push(req);
		me->stats[id]++;
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

void fast_cgi_multiple_threaded_app::setup(int num,int buffer,worker_thread **workers)
{
	int i;
	
	stats=NULL;
	requests=NULL;
	threads_info=NULL;
	pids=NULL;
	
	size=num;
	
	// Statistics
	
	stats = new long long [size];
	for(i=0;i<size;i++)
		stats[i]=0;
	
	// Init Worker Threads
	this->workers=workers;
	for(i=0;i<size;i++) {
		workers[i]->init();
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

bool fast_cgi_multiple_threaded_app::run()
{
	int res;
	if(wait()==ACCEPT) {
		FCGX_Request *req=requests_queue.pop();
		
		#ifdef FCGX_API_ACCEPT_ONLY_EXISTS
		res=FCGX_Accept_Only_r(req); 
		#else 
		res=FCGX_Accept_r(req);
		#endif
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
