#include "manager.h"
#include "cgicc_connection.h"
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include "scgi.h"
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "thread_cache.h"
#include "process_cache.h"

namespace cppcms {
namespace details {

fast_cgi_application *fast_cgi_application::handlers_owner=NULL;

fast_cgi_application::fast_cgi_application(cgi_api &a) : api(a)
{
}

fast_cgi_application::event_t fast_cgi_application::wait()
{
	while(!the_end) {
		struct pollfd fds;

		fds.fd=api.get_socket();
		fds.events=POLLIN | POLLERR;
		fds.revents=0;

		/* Wait for two events:
		 * 1. New connection and then do accept
		 * 2. Exit message - exit and do clean up */

		if(poll(&fds,1,-1)<0) {
			if(errno==EINTR && !the_end)
				continue;
			return EXIT;
		}
		else if(fds.revents) {
			if(fds.revents & POLLERR)
				return EXIT;
			return ACCEPT;
		}
	}
	return EXIT;
}

void fast_cgi_application::shutdown()
{
	// Rise exit signal on
	// the selfpipe
	the_end=true;
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

fast_cgi_single_threaded_app::fast_cgi_single_threaded_app(base_factory const &factory,cache_factory const &cf,cgi_api &a) :
	fast_cgi_application(a)
{
	worker=factory(cf);
}

bool fast_cgi_single_threaded_app::run()
{
	// Blocking loop
	event_t event=wait();

	if(event==EXIT) {
		return false;
	} // ELSE event==ACCEPT

	cgi_session *session=api.accept_session();
	if(session) {
		try {
			if(session->prepare()) {
				worker->run(session->get_connection());
			}
		}
		catch(...) {
			delete session;
			throw;
		}
		delete session;
	}
	return true;
};


void fast_cgi_application::execute()
{
	the_end=false;

	set_signal_handlers();

	while(run()){
		/* Do Continue */
	}
}

fast_cgi_multiple_threaded_app::fast_cgi_multiple_threaded_app(	int threads_num,
								int buffer,
								base_factory const &factory,cache_factory const &cf,
								cgi_api &a) :
	fast_cgi_application(a)
{
	int i;

	threads_info=NULL;
	pids=NULL;

	size=threads_num;

	// Init Worker Threads
	workers.resize(size);

	for(i=0;i<size;i++) {
		workers[i]=factory(cf);
	}

	// Setup Jobs Manager
	jobs.init(buffer);

	start_threads();
}

void *fast_cgi_multiple_threaded_app::thread_func(void *p)
{
	info_t *params=(info_t *)p;

	int id=params->first;
	fast_cgi_multiple_threaded_app *self=params->second;

	cgi_session *session;

	while((session=self->jobs.pop())!=NULL) {
		try{
			if(session->prepare()){
				self->workers[id]->run(session->get_connection());
			}
		}
		catch(...){
			delete session;
			return NULL;
		}
		delete session;
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

bool fast_cgi_multiple_threaded_app::run()
{
	if(wait()==ACCEPT) {
		cgi_session *session=api.accept_session();
		if(session){
			jobs.push(session);
		}
		return true;
	}
	else {// Exit event
		int i;
		for(i=0;i<size;i++) {
			jobs.push(NULL);
		}

		wait_threads();

		return false;
	}
}


// Single instance of prefork
prefork *prefork::self;

void prefork::parent_handler(int s_catched)
{
	int i;
	int s;
	if(self->exit_flag==0) {
		self->exit_flag=1;
		s=SIGTERM;
	}
	else {
		s=SIGKILL;
	}

	signal(SIGALRM,parent_handler);
	alarm(3);
	for(i=0;i<self->procs;i++) {
		kill(self->pids[i],s);
	}
}

void prefork::chaild_handler(int s)
{
	self->exit_flag=1;
	signal(SIGALRM,chaild_handler);
	alarm(1);
}

void prefork::run()
{
	signal(SIGTERM,chaild_handler);

	shared_ptr<worker_thread> worker=factory(cache);

	int res,post_on_throw;
	int limit=global_config.lval("server.iterations_limit",-1);
	if(limit!=-1) {
		srand(getpid());
		limit=limit+(limit / 10 *(rand() % 100))/100;
	}
	int counter=0;
	while(!exit_flag){
		if(limit!=-1 && counter>limit)
			return;
		counter++;
		res=sem_wait(semaphore);
		if(res<0){
			if(errno==EINTR)
				continue;
			else {
				perror("sem_wait");
				exit(1);
			}
		}
		cgi_session *session=NULL;
		try{
			post_on_throw=1;

			struct pollfd fds;
			fds.fd=api.get_socket();
			fds.revents=0;
			fds.events=POLLIN | POLLERR;

			if(poll(&fds,1,-1)==1 && (fds.revents & POLLIN)) {
				session=api.accept_session();
			}
			post_on_throw=0;
			sem_post(semaphore);
			if(session && session->prepare()) {
				worker->run(session->get_connection());
			}
		}
		catch(cppcms_error const &e){
			if(post_on_throw) sem_post(semaphore);
			cerr<<e.what();
		}
		catch(...){
			if(post_on_throw) sem_post(semaphore);
			exit(1);
		}
		delete session;
	}
}

void prefork::execute()
{
	int i;
	void *mem=mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE,
			MAP_SHARED |MAP_ANONYMOUS,-1,0);
	if(mem==MAP_FAILED) {
		throw cppcms_error(errno,"mmap failed");
	}
	semaphore=(sem_t*)mem;
	sem_init(semaphore,1,1);

	for(i=0;i<procs;i++) {
		pid_t pid=fork();
		if(pid<0) {
			perror("fork:");
			int j;
			for(j=0;j<i;j++) {
				kill(pids[j],SIGKILL);
				wait(NULL);
			}
			exit(1);
		}
		if(pid>0) {
			pids[i]=pid;
		}
		else { // pid==0
			run();
			return;
		}
	}
	/* Signals defined by standard */
	signal(SIGTERM,parent_handler);
	signal(SIGUSR1,parent_handler);
	/* Additional signal */
	signal(SIGINT,parent_handler);

	while(!prefork::exit_flag) {
		int stat;
		pid_t pid=wait(&stat);
		if(pid<0) {
			continue;
		}
		if(exit_flag) break;
		for(i=0;i<procs;i++) {
			if(pids[i]==pid) {
				if(!WIFEXITED(stat) || WEXITSTATUS(stat)!=0){
					if(WIFEXITED(stat)) {
						cerr<<"Chaild "<<pid<<" exited with "<<WEXITSTATUS(stat)<<endl;
					}
					else if(WIFSIGNALED(stat)) {
						cerr<<"Chaild "<<pid<<" killed by "<<WTERMSIG(stat)<<endl;
					}
					else {
						cerr<<"Chaild "<<pid<<" exited for unknown reason"<<endl;
					}
				}
				pid=fork();
				if(pid==0) {
					run();
					return;
				}
				pids[i]=pid;
				break;
			}
		}
	}

	for(i=0;i<procs;i++)  {
		while(wait(NULL)<0 && errno==EINTR)
			;
	}
	sem_close(semaphore);
	munmap(mem,sizeof(sem_t));
}


} // END oF Details

static cache_factory *get_cache_factory()
{
	string backend=global_config.sval("cache.backend","none");

	if(backend=="none") {
		return new cache_factory();
	}
	else if(backend=="threaded") {
		int n=global_config.lval("cache.limit",100);
		return new thread_cache_factory(n);
	}
	else if(backend=="fork") {
		size_t s=global_config.lval("cache.memsize",64);
		string f=global_config.sval("cache.file","");
		return new process_cache_factory(s*1024U,f=="" ? NULL: f.c_str());
	}
	else {
		throw cppcms_error("Unkown cache backend:" + backend);
	}
}

void run_application(int argc,char *argv[],base_factory const &factory)
{
	int n,max;
	global_config.load(argc,argv);

	string api=global_config.sval("server.api") ;

	auto_ptr<cache_factory> cf(get_cache_factory());

	if(api=="cgi") {
		shared_ptr<worker_thread> ptr=factory(*cf);
		cgicc_connection_cgi cgi;
		ptr->run(cgi);
	}
	else
	{
		auto_ptr<cgi_api> capi;
		if(api=="fastcgi" || api=="scgi" ) {
			string socket=global_config.sval("server.socket","");
			int backlog=global_config.lval("server.buffer",1);
			if(api=="fastcgi")
				capi=auto_ptr<cgi_api>(new fcgi_api(socket.c_str(),backlog));
			else
				capi=auto_ptr<cgi_api>(new scgi_api(socket.c_str(),backlog));
		}
		else {
			throw cppcms_error("Unknown api:"+api);
		}

		string mod=global_config.sval("server.mod");
		if(mod=="process") {
			details::fast_cgi_single_threaded_app app(factory,*cf.get(),*capi.get());
			app.execute();
		}
		else if(mod=="thread") {
			n=global_config.lval("server.threads",5);
			max=global_config.lval("server.buffer",1);
			details::fast_cgi_multiple_threaded_app app(n,max,factory,*cf.get(),*capi.get());
			app.execute();
		}
		else if(mod=="prefork") {
			n=global_config.lval("server.procs",5);
			details::prefork app(factory,*cf.get(),*capi.get(),n);
			app.execute();
		}
		else {
			throw cppcms_error("Unknown mod:" + mod);
		}
	}
};




} // END OF CPPCMS
