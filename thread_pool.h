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
	FastCGI_Application(char const *socket,int backlog);
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
		: FastCGI_Application(socket,1) { setup(worker); };
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
class Safe_Set  {
	pthread_mutex_t access_mutex;
	pthread_cond_t new_data_availible;
	pthread_cond_t new_space_availible;
protected:
	int max;
	int size;
	virtual T &get_int() = 0;
	virtual void put_int(T &val) = 0;
public:
	void init(int size){
		pthread_mutex_init(&access_mutex,NULL);
		pthread_cond_init(&new_data_availible,NULL);
		pthread_cond_init(&new_space_availible,NULL);

		max=size;
		this->size=0;

	};
	Safe_Set() {};
	virtual ~Safe_Set() {};
	virtual void push(T val) {
		pthread_mutex_lock(&access_mutex);
		while(size>=max) {
			pthread_cond_wait(&new_space_availible,&access_mutex);			
		}
		put_int(val);
		pthread_cond_signal(&new_data_availible);
		pthread_mutex_unlock(&access_mutex);
	};
	T pop() {
		pthread_mutex_lock(&access_mutex);
		while(size==0) {
			pthread_cond_wait(&new_data_availible,&access_mutex);
		}
		T data=get_int();
		pthread_cond_signal(&new_space_availible);
		pthread_mutex_unlock(&access_mutex);
		return data;
	};
};

template <class T>
class Safe_Queue : public Safe_Set<T>{
	T *queue;
	int head;
	int tail;
	int next(int x) { return (x+1)%this->max; };
protected:
	virtual void put_int(T &val) {
		queue[head]=val;
		head=next(head);
		this->size++;
	}
	virtual T &get_int() {
		this->size--;
		int ptr=tail;
		tail=next(tail);
		return queue[ptr];
	}
public:
	void init(int size) {
		if(queue) return;
		queue=new T [size];
		Safe_Set<T>::init(size);
	}
	Safe_Queue() { queue = NULL; head=tail=0; };
	virtual ~Safe_Queue() { delete [] queue; };
};

class FastCGI_Mutiple_Threaded_App : public FastCGI_Application {
	int size;
	long long int *stats;
	Worker_Thread **workers;
	FCGX_Request  *requests;
	Safe_Queue<FCGX_Request*> requests_queue;
	Safe_Queue<FCGX_Request*> jobs_queue;
	
	void setup(int size,int buffer_len,Worker_Thread **workers);
	
	pthread_t *pids;

	typedef pair<int,FastCGI_Mutiple_Threaded_App*> info_t;
	
	info_t *threads_info;

	static void *thread_func(void *p);
	void start_threads();
	void wait_threads();
public:
	FastCGI_Mutiple_Threaded_App(	int num,
					int buffer_len,
					Worker_Thread **workers,
					char const *socket=NULL) :
		FastCGI_Application(socket,num+1+buffer_len)
	{
		setup(num,num+1+buffer_len,workers);
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
	FastCGI_MT(int num,int buffer, char const *socket) : 
		FastCGI_Mutiple_Threaded_App(num,buffer,setptrs(num),socket) {};
	virtual ~FastCGI_MT() {
		delete [] ptrs;
		delete [] threads;
	};
};

#endif /* _THREAD_PULL_H */
