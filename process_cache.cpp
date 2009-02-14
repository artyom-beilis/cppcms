#include "process_cache.h"
#include <boost/format.hpp>
#include <unistd.h>

#ifdef HAVE_PTHREADS_PSHARED
# include "posix_mutex.h"
#else
# include "fcntl_mutex.h"
using namespace cppcms::fcntl;
#endif

#include <errno.h>
#include <iostream>

using boost::format;
using boost::str;

namespace cppcms {

shmem_control *process_cache_factory::mem(NULL);
::pid_t process_cache_factory::owner_pid(0);


process_cache_factory::process_cache_factory(size_t memsize,char const *file)
{
	cache=NULL;
	if(memsize<8*1024) {
		throw cppcms_error("Cache size too small -- need at least 8K");
	}
	if(!mem) {
		mem=new shmem_control(memsize,file);
		owner_pid=getpid();
	}
	else {
		throw cppcms_error("The memory initilized -- can't use more then once cache in same time");
	}
	cache=new process_cache(memsize);
};

process_cache_factory::~process_cache_factory()
{
	// Only parent process can kill memory
	// forked childs should never do it.
	if(owner_pid==getpid()) {
		delete cache;
		delete mem;
 		mem=NULL;
	}
}

base_cache *process_cache_factory::get() const
{
	 return cache;
};
void process_cache_factory::del(base_cache *p) const
{
};

process_cache::process_cache(size_t m) :
			memsize(m)
{
#ifdef HAVE_PTHREADS_PSHARED
	pthread_mutexattr_t a;
	pthread_rwlockattr_t al;

	if(
		pthread_mutexattr_init(&a)
		|| pthread_mutexattr_setpshared(&a,PTHREAD_PROCESS_SHARED)
		|| pthread_mutex_init(&lru_mutex,&a)
		|| pthread_mutexattr_destroy(&a)
		|| pthread_rwlockattr_init(&al)
		|| pthread_rwlockattr_setpshared(&al,PTHREAD_PROCESS_SHARED)
		|| pthread_rwlock_init(&access_lock,&al)
		|| pthread_rwlockattr_destroy(&al))
	{
		throw cppcms_error(errno,"Failed setup mutexes --- is this system "
					 "supports process shared mutex/rwlock?");
	}
#else
	if((lru_mutex=tmpfile())==NULL || (access_lock=tmpfile())==NULL) {
		throw cppcms_error(errno,"Failed to create temporary file");
	}
#endif
};


process_cache::~process_cache()
{
#ifdef HAVE_PTHREADS_PSHARED
	pthread_mutex_destroy(&lru_mutex);
	pthread_rwlock_destroy(&access_lock);
#else
	fclose(lru_mutex);
	fclose(access_lock);
#endif
}

process_cache::shr_string *process_cache::get(string const &key,set<string> *triggers)
{
	pointer p;
	time_t now;
	time(&now);
	if((p=primary.find(key.c_str()))==primary.end() || p->second.timeout->first < now) {
		return NULL;
	}
	if(triggers) {
		list<triggers_ptr>::iterator tp;
		for(tp=p->second.triggers.begin();tp!=p->second.triggers.end();tp++) {
			triggers->insert((*tp)->first.c_str());
		}
	}
	{
		mutex_lock lock(lru_mutex);
		lru.erase(p->second.lru);
		lru.push_front(p);
		p->second.lru=lru.begin();
	}
	return &(p->second.data);
}

namespace {
	template<typename T>
	T unaligned(T const *p)
	{
		T tmp;
		memcpy(&tmp,p,sizeof(T));
		return tmp;
	}
}


bool process_cache::fetch_page(string const &key,string &out,bool gzip)
{
	rwlock_rdlock lock(access_lock);
	shr_string *r=get(key,NULL);
	if(!r) return false;
	size_t size=r->size();
	size_t s;
	char const *ptr=r->c_str();
	if(size<sizeof(size_t) || (s=unaligned((size_t const *)ptr))>size-sizeof(size_t))
		return false;
	if(!gzip){
		out.assign(ptr+sizeof(size_t),s);
	}
	else {
		ptr+=s+sizeof(size_t);
		size-=s+sizeof(size_t);
		if(size<sizeof(size_t) || (s=unaligned((size_t const *)ptr))!=size-sizeof(size_t))
			return false;
		out.assign(ptr+sizeof(size_t),s);
	}
	return true;
}

bool process_cache::fetch(string const &key,archive &a,set<string> &tags)
{
	rwlock_rdlock lock(access_lock);
	shr_string *r=get(key,&tags);
	if(!r) return false;
	a.set(r->c_str(),r->size());
	return true;
}

void process_cache::clear()
{
	rwlock_wrlock lock(access_lock);
	timeout.clear();
	lru.clear();
	primary.clear();
	triggers.clear();
}
void process_cache::stats(unsigned &keys,unsigned &triggers)
{
	rwlock_rdlock lock(access_lock);
	keys=primary.size();
	triggers=this->triggers.size();
}

void process_cache::rise(string const &trigger)
{
	rwlock_wrlock lock(access_lock);
	pair<triggers_ptr,triggers_ptr> range=triggers.equal_range(trigger.c_str());
	triggers_ptr p;
	list<pointer> kill_list;
	for(p=range.first;p!=range.second;p++) {
		kill_list.push_back(p->second);
	}
	list<pointer>::iterator lptr;

	for(lptr=kill_list.begin();lptr!=kill_list.end();lptr++) {
		delete_node(*lptr);
	}
}

void process_cache::store(string const &key,set<string> const &triggers_in,time_t timeout_in,archive const &a)
{
	rwlock_wrlock lock(access_lock);
	pointer main;
	main=primary.find(key.c_str());

	if(main!=primary.end())
		delete_node(main);

	if(a.get().size()>memsize/20) {
		return;
	}

	time_t now;
	time(&now);
	// Make sure there is at least 10% avalible
	// And there is a block that is big enough to allocate 5% of memory
	for(;;) {
		if(process_cache_factory::mem->available() > memsize / 10) {
			void *p=process_cache_factory::mem->malloc(memsize/20);
			if(p) {
				process_cache_factory::mem->free(p);
				break;
			}
		}
		if(timeout.begin()->first<now) {
			main=timeout.begin()->second;
		}
		else {
			main=lru.back();
		}
		delete_node(main);
	}

	try {
		pair<pointer,bool> res=primary.insert(pair<shr_string,container>(key.c_str(),container()));

		main=res.first;
		container &cont=main->second;
		cont.data.assign(a.get().c_str(),a.get().size());

		lru.push_front(main);
		cont.lru=lru.begin();
		cont.timeout=timeout.insert(pair<time_t,pointer>(timeout_in,main));
		if(triggers_in.find(key)==triggers_in.end()){
			cont.triggers.push_back(triggers.insert(
					pair<shr_string,pointer>(key.c_str(),main)));
		}
		set<string>::const_iterator si;
		for(si=triggers_in.begin();si!=triggers_in.end();si++) {
			cont.triggers.push_back(triggers.insert(
				pair<shr_string,pointer>(si->c_str(),main)));
		}
	}
	catch(std::bad_alloc const &e) {
		clear();
	}
}

void process_cache::delete_node(pointer p)
{
	lru.erase(p->second.lru);
	timeout.erase(p->second.timeout);
	list<triggers_ptr>::iterator i;
	for(i=p->second.triggers.begin();i!=p->second.triggers.end();i++) {
		triggers.erase(*i);
	}
	primary.erase(p);
}


void *process_cache::operator new(size_t n) {
	void *p=process_cache_factory::mem->malloc(n);
	if(!p)
		throw std::bad_alloc();
	return p;
}
void process_cache::operator delete (void *p) {
	process_cache_factory::mem->free(p);
}



};
