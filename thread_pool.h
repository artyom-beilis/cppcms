#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <pthread.h>
#include <string>
#include <memory>

#include "worker_thread.h"
#include "global_config.h"

namespace cppcms {

namespace details {

class fast_cgi_application {

	fast_cgi_application static  *handlers_owner;
protected:	
	// General control 
	
	int main_fd; //File descriptor associated with socket 
	int signal_pipe[2]; // Notification pipe
	
	std::string socket;

	static void handler(int id);
	
	typedef enum { EXIT , ACCEPT } event_t;
	event_t wait();
	void set_signal_handlers();
	static fast_cgi_application *get_instance() { return handlers_owner; };
public:
	fast_cgi_application(char const *socket,int backlog);
	virtual ~fast_cgi_application() {};

	void shutdown();
	virtual bool run() { return false; };
	void execute();
};

class fast_cgi_single_threaded_app : public fast_cgi_application {
	/* Single thread model -- one process runs */
	FCGX_Request request;	
	worker_thread *worker;
	void setup(worker_thread *worker);
public:
	virtual bool run();
	fast_cgi_single_threaded_app(worker_thread *worker,char const *socket=NULL)
		: fast_cgi_application(socket,1) { setup(worker); };
	virtual ~fast_cgi_single_threaded_app(){};
};


template <class WT>
class fast_cgi_st : public fast_cgi_single_threaded_app {
	worker_thread *wt;
public:
	fast_cgi_st(char const *socket=NULL) : 
		fast_cgi_single_threaded_app((wt=new WT) ,socket) {};
	virtual ~fast_cgi_st(){ delete wt; };
};

template <class T>
class sefe_set  {
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
	sefe_set() {};
	virtual ~sefe_set() {};
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
class sefe_queue : public sefe_set<T>{
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
		sefe_set<T>::init(size);
	}
	sefe_queue() { queue = NULL; head=tail=0; };
	virtual ~sefe_queue() { delete [] queue; };
};

class fast_cgi_multiple_threaded_app : public fast_cgi_application {
	int size;
	long long int *stats;
	worker_thread **workers;
	FCGX_Request  *requests;
	sefe_queue<FCGX_Request*> requests_queue;
	sefe_queue<FCGX_Request*> jobs_queue;
	
	void setup(int size,int buffer_len,worker_thread **workers);
	
	pthread_t *pids;

	typedef pair<int,fast_cgi_multiple_threaded_app*> info_t;
	
	info_t *threads_info;

	static void *thread_func(void *p);
	void start_threads();
	void wait_threads();
public:
	fast_cgi_multiple_threaded_app(	int num,
					int buffer_len,
					worker_thread **workers,
					char const *socket=NULL) :
		fast_cgi_application(socket,num+1+buffer_len)
	{
		setup(num,num+1+buffer_len,workers);
	};
	virtual bool run();	
	virtual ~fast_cgi_multiple_threaded_app() {
		delete [] requests;
		delete [] pids;
		delete [] threads_info;
		delete [] stats;
	};
	long long int const *get_stats() { return stats; };
};

template<class WT>
class fast_cgi_mt : public fast_cgi_multiple_threaded_app {
	WT *threads;
	worker_thread **ptrs;
	
	worker_thread **setptrs(int num)
	{
		threads = new  WT [num];
		ptrs= new worker_thread* [num] ;
		for(int i=0;i<num;i++) {
			ptrs[i]=threads+i;
		}
		return ptrs;
	};
public:
	fast_cgi_mt(int num,int buffer, char const *socket) : 
		fast_cgi_multiple_threaded_app(num,buffer,setptrs(num),socket) {};
	virtual ~fast_cgi_mt() {
		delete [] ptrs;
		delete [] threads;
	};
};

} // END OF DETAILS

template<class T>
void run_application(int argc,char *argv[])
{
	using namespace details;
	
	int n,max;
	global_config.load(argc,argv);
	
	char const *socket=global_config.sval("server.socket","").c_str();
	
	if((n=global_config.lval("server.threads",0))==0) {
		auto_ptr<fast_cgi_st<T> > app(new fast_cgi_st<T>(socket));
		app->execute();
	}
	else {
		max=global_config.lval("server.buffer",1);
		auto_ptr<fast_cgi_mt<T> > app(new fast_cgi_mt<T>(n,max,socket));
		app->execute();
	}
};

}
#endif /* _THREAD_POOL_H */
