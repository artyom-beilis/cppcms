#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "manager.h"
#include "cgicc_connection.h"
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "thread_cache.h"
#include "scgi.h"
#include "cgi.h"

#ifdef EN_FORK_CACHE
# include "process_cache.h"
#endif

#ifdef EN_FCGI_BACKEND
# include "fcgi.h"
#endif

#ifdef EN_TCP_CACHE
# include "tcp_cache.h"
#endif

namespace cppcms {
namespace details {

class single_run : public web_application{
public:
	single_run(manager &m) : web_application(m)
	{
	};
	virtual void execute()
	{
		base_factory &factory=*app.workers;
		shared_ptr<worker_thread> worker(factory(app));
		cgi_session *session=app.api->accept_session();
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

	}
};

fast_cgi_application *fast_cgi_application::handlers_owner=NULL;

fast_cgi_application::event_t fast_cgi_application::wait()
{
	while(!the_end) {
		struct pollfd fds;

		fds.fd=app.api->get_socket();
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

fast_cgi_single_threaded_app::fast_cgi_single_threaded_app(manager &m) :
		fast_cgi_application( m)
{
	base_factory &factory=*app.workers;
	worker=factory(app);
}

bool fast_cgi_single_threaded_app::run()
{
	// Blocking loop
	event_t event=wait();

	if(event==EXIT) {
		return false;
	} // ELSE event==ACCEPT

	cgi_session *session=app.api->accept_session();
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

fast_cgi_multiple_threaded_app::fast_cgi_multiple_threaded_app(manager &m):
	fast_cgi_application(m)
{
	int threads_num=app.config.lval("server.threads",5);
	int buffer=app.config.lval("server.buffer",10);

	int i;

	threads_info=NULL;
	pids=NULL;

	size=threads_num;

	// Init Worker Threads
	workers.resize(size);

	base_factory &factory=*app.workers;
	for(i=0;i<size;i++) {
		workers[i]=factory(app);
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
		cgi_session *session=app.api->accept_session();
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

	base_factory &factory=*app.workers;
	shared_ptr<worker_thread> worker=factory(app);

	int res,post_on_throw;
	int limit=app.config.lval("server.iterations_limit",-1);
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
			fds.fd=app.api->get_socket();
			fds.revents=0;
			fds.events=POLLIN | POLLERR;

			if(poll(&fds,1,-1)==1 && (fds.revents & POLLIN)) {
				session=app.api->accept_session();
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

prefork::prefork(manager &m) :
	web_application(m)
{
	procs=app.config.lval("server.procs",5);
	exit_flag=0;
	pids.resize(procs);
	self=this;
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

cache_factory *manager::get_cache_factory()
{
	string backend=config.sval("cache.backend","none");

	if(backend=="none") {
		return new cache_factory();
	}
	else if(backend=="threaded") {
		int n=config.lval("cache.limit",100);
		return new thread_cache_factory(n);
	}
#ifdef EN_FORK_CACHE
	else if(backend=="fork") {
		size_t s=config.lval("cache.memsize",64);
		string f=config.sval("cache.file","");
		return new process_cache_factory(s*1024U,f=="" ? NULL: f.c_str());
	}
#endif
#ifdef EN_TCP_CACHE
	else if(backend=="tcp") {
		vector<long> const &ports=config.llist("cache.ports");
		vector<string> const &ips=config.slist("cache.ips");
		return new tcp_cache_factory(ips,ports);
	}
#endif
	else {
		throw cppcms_error("Unkown cache backend:" + backend);
	}
}

cgi_api *manager::get_api()
{
	string api=config.sval("server.api");

	if(api=="cgi") {
		return new cgi_cgi_api();
	}

	string socket=config.sval("server.socket","");
	int backlog=config.lval("server.buffer",1);

	if(api=="scgi" ) {
		return new scgi_api(socket.c_str(),backlog);
	}

#ifdef EN_FCGI_BACKEND
	if(api=="fastcgi"){
		return new fcgi_api(socket.c_str(),backlog);
	}
#endif
	throw cppcms_error("Unknown api:"+api);

}

web_application *manager::get_mod()
{
	if(config.sval("server.api","")=="cgi") {
		return new details::single_run(*this);
	}

	string mod=config.sval("server.mod");

	if(mod=="process") {
		return new details::fast_cgi_single_threaded_app(*this);
	}
	if(mod=="thread") {
		return new details::fast_cgi_multiple_threaded_app(*this);
	}
	if(mod=="prefork") {
		return new details::prefork(*this);
	}
	throw cppcms_error("Unknown mod:" + mod);
}

void manager::execute()
{
	if(!cache.get()) {
		set_cache(get_cache_factory());
	}
	if(!api.get()) {
		set_api(get_api());
	}
	if(!web_app.get()) {
		set_mod(get_mod());
	}
	if(!workers.get()) {
		throw cppcms_error("No workers factory set up");
	}
	web_app->execute();
}

void manager::set_worker(base_factory *w)
{
	workers=auto_ptr<base_factory>(w);
}

void manager::set_cache(cache_factory *c)
{
	cache=auto_ptr<cache_factory>(c);
}

void manager::set_api(cgi_api *a)
{
	api=auto_ptr<cgi_api>(get_api());
}

void manager::set_mod(web_application *m)
{
	web_app=auto_ptr<web_application>(m);
}

manager::manager()
{
	config.load(0,NULL);
}

manager::manager(char const *f)
{
	config.load(0,NULL,f);
}

manager::manager(int argc, char **argv)
{
	config.load(argc,argv);
}


} // END OF CPPCMS
