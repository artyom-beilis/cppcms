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
		main_fd=FCGX_OpenSocket(socket,1);
		if(main_fd<0) {
			throw HTTP_Error(string("Fauled to open socket ")
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


void FastCGI_Single_Threaded_App::execute()
{
	set_signal_handlers();
	
	while(run()){
		/* Do Continue */
	}
}

//FastCGI_Application::handlers_owner(0);
