#ifndef CPPCMS_MANAGER_H
#define CPPCMS_MANAGER_H

#include <pthread.h>
#include <string>
#include <memory>

#include "worker_thread.h"
#include "global_config.h"
#include "cache_interface.h"
#include "cgi_api.h"
#include "posix_mutex.h"
#include "transtext.h"
#include "session_backend_factory.h"
#include <boost/shared_ptr.hpp>

namespace cppcms {

class manager;

using boost::shared_ptr;

class base_factory {
public:
	virtual shared_ptr<worker_thread> operator()(manager const &cf) const = 0;
	virtual ~base_factory() {};
};

template<typename T>
class simple_factory : public base_factory {
public:
	virtual shared_ptr<worker_thread> operator()(manager const &cf) const
	{ return shared_ptr<worker_thread>(new T(cf)); };
};

template<typename T,typename P>
class one_param_factory : public base_factory {
	P const &P1;
public:
	one_param_factory(P const &p) : P1(p) {};
	virtual shared_ptr<worker_thread> operator()(manager const &cf) const
	{ return shared_ptr<worker_thread>(new T(cf,P1)); };
};

template<typename T,typename Pa,typename Pb>
class two_params_factory : public base_factory {
	Pa const &P1;
	Pb const &P2;
public:
	two_params_factory(Pa const &p1,Pb const &p2) : P1(p1), P2(p2) {};
	virtual shared_ptr<worker_thread> operator()(manager const &cf) const
	{ return shared_ptr<worker_thread>(new T(cf,P1,P2)); };
};

class web_application {
public:
	manager &app;
	web_application(manager &m) : app(m) {};
	virtual void execute() = 0;
	virtual ~web_application() {};
};


namespace details {
#if !defined(CPPCMS_EMBEDDED) || defined(CPPCMS_EMBEDDED_THREAD)
class fast_cgi_application :public web_application {

	static fast_cgi_application  *handlers_owner;
protected:
	// General control

	bool the_end;

	static void handler(int id);

	typedef enum { EXIT , ACCEPT } event_t;
	virtual event_t wait();
	void set_signal_handlers();
	static fast_cgi_application *get_instance() { return handlers_owner; };
public:
	fast_cgi_application(manager &m) : web_application(m) {};
	virtual ~fast_cgi_application() {};

	void shutdown();
	virtual bool run() { return false; };
	virtual void execute();
};

class fast_cgi_single_threaded_app : public fast_cgi_application {
	shared_ptr<worker_thread> worker;
	void setup();
public:
	virtual bool run();
	fast_cgi_single_threaded_app(manager &m);
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
		mutex_lock lock(access_mutex);
		while(size>=max) {
			pthread_cond_wait(&new_space_availible,&access_mutex);
		}
		put_int(val);
		pthread_cond_signal(&new_data_availible);
	};
	T pop() {
		mutex_lock lock(access_mutex);
		while(size==0) {
			pthread_cond_wait(&new_data_availible,&access_mutex);
		}

		T data=get_int();
		pthread_cond_signal(&new_space_availible);
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
	sefe_queue<cgi_session *> jobs;
	typedef pair<int,fast_cgi_multiple_threaded_app*> info_t;

	info_t *threads_info;
	pthread_t *pids;

	static void *thread_func(void *p);
	void start_threads();
	void wait_threads();
public:
	fast_cgi_multiple_threaded_app(manager &m);
	virtual bool run();
	virtual ~fast_cgi_multiple_threaded_app() {
		delete [] pids;
		delete [] threads_info;
	};
};

#endif

#if !defined(CPPCMS_EMBEDDED) 

class prefork : public web_application {
	vector<pid_t> pids;
	int procs;
	int exit_flag;
	static prefork *self;
	static void parent_handler(int s);
	static void chaild_handler(int s);
	void run();
public:
	prefork(manager &m);
	virtual void execute();
};

#endif

} // END OF DETAILS

void run_application(int argc,char *argv[],base_factory const &factory);

class manager : private boost::noncopyable {
	cache_factory *get_cache_factory();
	cgi_api *get_api();
	web_application *get_mod();
	session_backend_factory get_sessions();
	transtext::trans_factory *get_gettext();
	list<void *> templates_list;
	void load_templates();
public:
	cppcms_config config;
	auto_ptr<cache_factory> cache;
	auto_ptr<cgi_api> api;
	auto_ptr<base_factory> workers;
	auto_ptr<web_application> web_app;
	session_backend_factory sessions;
	auto_ptr<transtext::trans_factory> gettext;

	void set_worker(base_factory *w);
	void set_cache(cache_factory *c);
	void set_api(cgi_api *a);
	void set_mod(web_application *m);
	void set_gettext(transtext::trans_factory *);
	void set_sessions(session_backend_factory);

	manager();
	manager(char const *file);
	manager(int argc, char **argv);
	~manager();
	void execute();
};

}
#endif /* CPPCMS_MANAGER_H */
