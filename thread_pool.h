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
	void execute();
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
};


template <class WT>
class FastCGI_ST : public FastCGI_Single_Threaded_App {
	Worker_Thread *worker_thread;
public:
	FastCGI_ST(char const *socket=NULL) : 
		FastCGI_Single_Threaded_App((worker_thread=new WT) ,socket) {};
	virtual ~FastCGI_ST(){ delete worker_thread; };
};

template <class T>
class Safe_Stack {
	T *stack;
	int stack_size;
	int max_size;
	pthread_mutex_t access_mutex;
	pthread_cond_t new_data_availible;
	pthread_cond_t new_space_availible;
public:
	void init(int size){
		if(stack!=NULL)
			return;
		pthread_mutex_init(&access_mutex,NULL);
		pthread_cond_init(&new_data_availible,NULL);
		pthread_cond_init(&new_space_availible,NULL);
		stack = new T [size];
		max_size=size;
		stack_size=0;
	};
	Safe_Stack() {
		stack=NULL;
	};
	~Safe_Stack() {
		delete [] stack;
	};
	void push(T val) {
		pthread_mutex_lock(&access_mutex);
		while(stack_size>=max_size) {
			pthread_cond_wait(&new_space_availible,&access_mutex);			
		}
		stack[stack_size]=val;
		stack_size++;
		pthread_cond_signal(&new_data_availible);
		pthread_mutex_unlock(&access_mutex);
	};
	T pop() {
		pthread_mutex_lock(&access_mutex);
		while(stack_size==0) {
			pthread_cond_wait(&new_data_availible,&access_mutex);
		}
		stack_size--;
		T data=stack[stack_size];
		pthread_cond_signal(&new_space_availible);
		pthread_mutex_unlock(&access_mutex);
		return data;
	};
};

class FastCGI_Mutiple_Threaded_App : public FastCGI_Application {
	int size;
	long long int *stats;
	Worker_Thread **workers;
	FCGX_Request  *requests;
	Safe_Stack<FCGX_Request*> requests_stack;
	Safe_Stack<FCGX_Request*> jobs_stack;
	
	void setup(int size,Worker_Thread **workers);
	
	pthread_t *pids;

	typedef pair<int,FastCGI_Mutiple_Threaded_App*> info_t;
	
	info_t *threads_info;

	static void *thread_func(void *p);
	void start_threads();
	void wait_threads();
public:
	FastCGI_Mutiple_Threaded_App(	int num,
					Worker_Thread **workers,
					char *socket=NULL) :
		FastCGI_Application(socket)
	{
		setup(num,workers);
	};
	virtual bool run();	
	virtual ~FastCGI_Mutiple_Threaded_App() {
		delete [] requests;
		delete [] pids;
		delete [] threads_info;
		delete [] stats;
	};
	long long int const *get_stats() { return stats; };
};

template<class WT>
class FastCGI_MT : public FastCGI_Mutiple_Threaded_App {
	WT *threads;
	Worker_Thread **ptrs;
	
	Worker_Thread **setptrs(int num)
	{
		threads = new  WT [num];
		ptrs= new Worker_Thread* [num] ;
		for(int i=0;i<num;i++) {
			ptrs[i]=threads+i;
		}
		return ptrs;
	};
public:
	FastCGI_MT(int num, char *socket) : 
		FastCGI_Mutiple_Threaded_App(num,setptrs(num),socket) {};
	virtual ~FastCGI_MT() {
		delete [] ptrs;
		delete [] threads;
	};
};

#endif /* _THREAD_PULL_H */
