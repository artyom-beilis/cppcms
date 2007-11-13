#include "thread_pool.h"

#include <poll.h>
#include <signal.h>


FastCGI_Application *FastCGI_Application::handlers_owner=NULL;


FastCGI_Application::FastCGI_Application(const char *socket)
{
	FCGX_Init();
	
	pipe(signal_pipe);
	if(socket==NULL){
		main_fd=0;
	}
	else {
		this->socket=socket;
		main_fd=FCGX_OpenSocket(socket,50);
		if(main_fd<0) {
			throw HTTP_Error(string("Failed to open socket ")
					+socket);
		}
	}
}


FastCGI_Application::event_t FastCGI_Application::wait()
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

void FastCGI_Application::shutdown()
{
	// Rise exit signal on
	// the selfpipe
	write(signal_pipe[1],"0",1);
}

void FastCGI_Application::handler(int id)
{
	FastCGI_Application *ptr=FastCGI_Application::get_instance();
	if(ptr) {
		ptr->shutdown();
	}
}

void FastCGI_Application::set_signal_handlers()
{
	handlers_owner=this;
	/* Signals defined by standard */
	signal(SIGTERM,handler);
	signal(SIGUSR1,handler);
	/* Additional signal */
	signal(SIGINT,handler);
}
void FastCGI_Single_Threaded_App::setup(Worker_Thread *worker)
{
	this->worker=worker;
	worker->init();
	FCGX_InitRequest(&request, main_fd, 0);
}

bool FastCGI_Single_Threaded_App::run()
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


void FastCGI_Application::execute()
{
	set_signal_handlers();
	
	while(run()){
		/* Do Continue */
	}
}

void *FastCGI_Mutiple_Threaded_App::thread_func(void *p)
{
	info_t *params=(info_t *)p;

	int id=params->first;
	FastCGI_Mutiple_Threaded_App *me=params->second;

	FCGX_Request *req;
	
	while((req=me->jobs_stack.pop())!=NULL) {
		me->workers[id]->run(req);
		me->requests_stack.push(req);
		me->stats[id]++;
	}
	return NULL;
}

void FastCGI_Mutiple_Threaded_App::start_threads()
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

void FastCGI_Mutiple_Threaded_App::wait_threads()
{
	int i;
	for(i=0;i<size;i++) {
		pthread_join(pids[i],NULL);
	}
}

void FastCGI_Mutiple_Threaded_App::setup(int num,Worker_Thread **workers)
{
	int i;
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
	requests = new FCGX_Request [size+1];
	for(i=0;i<size;i++) {
		FCGX_InitRequest(requests+i, main_fd, 0);
	}
	requests_stack.init(size+1);
	for(i=0;i<size;i++)
		requests_stack.push(requests+i);
	
	// Setup Jobs Manager
	jobs_stack.init(1);

	start_threads();	
}

extern int global_counter;

bool FastCGI_Mutiple_Threaded_App::run()
{
	int res;
	if(wait()==ACCEPT) {
		FCGX_Request *req=requests_stack.pop();
		
		#ifdef FCGX_API_ACCEPT_ONLY_EXISTS
		res=FCGX_Accept_Only_r(req); 
		#else 
		res=FCGX_Accept_r(req);
		#endif
		if(res<0) {
			requests_stack.push(req);
			return true;
		}
		global_counter++;
		jobs_stack.push(req);
		return true;
	}
	else {// Exit event 
		int i;
		for(i=0;i<size;i++) {
			jobs_stack.push(NULL);
		}

		wait_threads();
		
		return false;
	}
}
