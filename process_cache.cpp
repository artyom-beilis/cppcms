#define CPPCMS_SOURCE
#include "process_cache.h"
#include "config.h"

#include <errno.h>
#include <iostream>

namespace cppcms {
namespace impl {


class process_cache_impl : public base_cache {
	mutex &lru_mutex;
	shared_mutex &access_lock;

	typedef std::basic_string<char,std::char_traits<char>,shmem_allocator<char,process_cache_factory::mem> > shr_string;
	struct container {
		shr_string data;
		typedef map<shr_string,container,std::less<shr_string>,
			shmem_allocator<std::pair<shr_string,container>,process_cache_factory::mem > > primary_map;
		typedef primary_map::iterator pointer;
		typedef list<pointer,shmem_allocator<pointer,process_cache_factory::mem> > pointer_list;
		pointer_list::iterator lru;
		typedef multimap<shr_string,pointer,std::less<shr_string>,
				shmem_allocator<std::pair<shr_string,pointer>,process_cache_factory::mem > > secondary_map;

		list<secondary_map::iterator,shmem_allocator<secondary_map::iterator,process_cache_factory::mem> > triggers;
		typedef multimap<time_t,pointer,std::less<time_t>,
				shmem_allocator<std::pair<time_t,pointer>,process_cache_factory::mem > > timeout_map;
		timeout_map::iterator timeout;
	};
	typedef container::pointer pointer;
	container::primary_map primary;
	container::secondary_map triggers;
	typedef container::secondary_map::iterator triggers_ptr;
	container::timeout_map timeout;
	typedef container::timeout_map::iterator timeout_ptr;
	container::pointer_list lru;
	typedef  container::pointer_list::iterator lru_ptr;
	unsigned memsize;

	shr_string *get(string const &key,set<string> *triggers);
	void delete_node(pointer p);
	int fd;

public:
	process_cache(size_t memsize,mutex &m,shared_mutex &a);
	virtual bool fetch(string const &key,string &a,set<string> *tags);
	virtual void rise(string const &trigger);
	virtual void clear();
	virtual void stats(unsigned &keys,unsigned &triggers);
	virtual void store(string const &key,set<string> const &triggers,time_t timeout,archive const &a);
	virtual ~process_cache();
	void *operator new(size_t n);
	void operator delete (void *p);
};


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

process_cache::process_cache(size_t s,mutex &m,shared_mutex &a) :
	memsize(s),
	lru_mutex(m),
	access_lock(a)
{
};


process_cache::~process_cache()
{
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
		mutex::guard lock(lru_mutex);
		lru.erase(p->second.lru);
		lru.push_front(p);
		p->second.lru=lru.begin();
	}
	return &(p->second.data);
}

bool process_cache::fetch(string const &key,archive &a,set<string> &tags)
{
	shared_mutex::shared_guard lock(access_lock);
	shr_string *r=get(key,&tags);
	if(!r) return false;
	a.set(r->c_str(),r->size());
	return true;
}

void process_cache::clear()
{
	shared_mutex::unique_guard lock(access_lock);
	timeout.clear();
	lru.clear();
	primary.clear();
	triggers.clear();
}
void process_cache::stats(unsigned &keys,unsigned &triggers)
{
	shared_mutex::shared_guard lock(access_lock);
	keys=primary.size();
	triggers=this->triggers.size();
}

void process_cache::rise(string const &trigger)
{
	shared_mutex::unique_guard lock(access_lock);
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
	shared_mutex::unique_guard lock(access_lock);
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
			try {
				void *p=process_cache_factory::mem->malloc(memsize/20);
				process_cache_factory::mem->free(p);
				break;
			}
			catch(std::bad_alloc const &e)
			{
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

class process_cache_holder : public base_cache {
public:
	process_cache_holder(size_t memory_size) 
	{
		lru_ = new mutex(true);
		access_ = new shared_mutex(true);
		impl_ = new process_cache(memory_size,*lru_,*access_);
		pid_ = ::getpid();
	}
	~process_cache_holder()
	{
		if(pid_ == ::getpid())
	}
	virtual bool fetch(std::string const &key,std::string &a,std::set<std::string> *tags)
	{
		return impl_->fetch(ket,a,tags);
	}
	virtual void store(std::string const &key,std::string const &b,std::set<std::string> const &triggers,time_t timeout)
	{
		return impl_->store(key,b,triggers,timeout);
	}
	virtual void rise(std::string const &trigger) 
	{
		return impl_->rise(trigger);
	}
	virtual void clear() 
	{
		return impl_->clear();
	}

	virtual void stats(unsigned &keys,unsigned &triggers)
	{
		return impl_->stats;
	}
	virtual ~base_cache()
	{
	}

	static mem-
};




} // impl
} // cppcms
