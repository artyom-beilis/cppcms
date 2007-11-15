#include "thread_pool.h"

#include <poll.h>
#include <signal.h>


FastCGI_Application *FastCGI_Application::handlers_owner=NULL;


FastCGI_Application::FastCGI_Application(const char *socket,int backlog)
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
	
	while((req=me->jobs_queue.pop())!=NULL) {
		me->workers[id]->run(req);
		me->requests_queue.push(req);
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

void FastCGI_Mutiple_Threaded_App::setup(int num,int buffer,Worker_Thread **workers)
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

bool FastCGI_Mutiple_Threaded_App::run()
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
