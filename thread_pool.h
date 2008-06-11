#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <pthread.h>
#include <string>
#include <memory>

#include "worker_thread.h"
#include "global_config.h"

#include <boost/shared_ptr.hpp>

namespace cppcms {

using boost::shared_ptr;

class base_factory {
public:
	virtual shared_ptr<worker_thread> operator()() const = 0;
	virtual ~base_factory() {};
};

template<typename T>
class simple_factory : public base_factory {
public:
	virtual shared_ptr<worker_thread> operator()() const { return shared_ptr<worker_thread>(new T()); };
};


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
	shared_ptr<worker_thread> worker;
	void setup();
public:
	virtual bool run();
	fast_cgi_single_threaded_app(base_factory const &factory,char const *socket=NULL);
	virtual ~fast_cgi_single_threaded_app(){};
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
	vector<shared_ptr<worker_thread> > workers;
	sefe_queue<FCGX_Request*> requests_queue;
	sefe_queue<FCGX_Request*> jobs_queue;
	typedef pair<int,fast_cgi_multiple_threaded_app*> info_t;
	
	FCGX_Request  *requests;
	info_t *threads_info;
	pthread_t *pids;

	

	static void *thread_func(void *p);
	void start_threads();
	void wait_threads();
public:
	fast_cgi_multiple_threaded_app(	int num,int buffer_len,	base_factory const &facory,char const *socket=NULL);
	virtual bool run();	
	virtual ~fast_cgi_multiple_threaded_app() {
		delete [] requests;
		delete [] pids;
		delete [] threads_info;
	};
};

} // END OF DETAILS

void run_application(int argc,char *argv[],base_factory const &factory);

}
#endif /* _THREAD_POOL_H */
