#ifndef _THREAD_PULL_H
#define _THREAD_PULL_H

#include <pthread.h>
#include <string>

#include "worker_thread.h"

#define MAX_THREADS_ALLOWED 256


class FastCGI_Application {

	FastCGI_Application static  *handlers_owner;
protected:	
	// General control 
	
	int main_fd; //File descriptor associated with socket 
	int signal_pipe[2]; // Notification pipe
	
	std::string socket;

	static void handler(int id);
	
	typedef enum { EXIT , ACCEPT } event_t;
	event_t wait();
	void set_signal_handlers();
	static FastCGI_Application *get_instance() { return handlers_owner; };
public:
	FastCGI_Application(char const *socket=NULL);
	virtual ~FastCGI_Application() {};

	void shutdown();
	virtual bool run() { return false; };
	virtual void execute() {};
};

class FastCGI_Single_Threaded_App : public FastCGI_Application {
	/* Single thread model -- one process runs */
	FCGX_Request request;	
	Worker_Thread *worker;
	void setup(Worker_Thread *worker);
public:
	virtual bool run();
	FastCGI_Single_Threaded_App(Worker_Thread *worker,char const *socket=NULL)
		: FastCGI_Application(socket) { setup(worker); };
	virtual ~FastCGI_Single_Threaded_App(){};
	virtual void execute();
};


#endif /* _THREAD_PULL_H */
