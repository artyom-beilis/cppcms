#ifndef CPPCMS_PROC_CHACHE_H
#define CPPCMS_PROC_CHACHE_H
#include "base_cache.h"
#include "cache_interface.h"
#include <pthread.h>
#include <map>
#include <list>
#include <sys/types.h>
#include "shmem_allocator.h"

namespace cppcms {

class process_cache;

class process_cache_factory : public cache_factory{
	process_cache *cache;
	static ::pid_t owner_pid;
public:
	static shmem_control *mem;
	process_cache_factory(size_t memsize,char const *file);
	virtual ~process_cache_factory();
	virtual base_cache *get() const;
	virtual void del(base_cache *p) const;
};

using namespace std;

class process_cache : public base_cache {
#ifdef HAVE_PTHREADS_PSHARED
	pthread_mutex_t lru_mutex;
	pthread_rwlock_t access_lock;
#else
	FILE *lru_mutex;
	FILE *access_lock;
#endif
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
	process_cache(size_t memsize);
	virtual bool fetch_page(string const &key,string &output,bool gzip);
	virtual bool fetch(string const &key,archive &a,set<string> &tags);
	virtual void rise(string const &trigger);
	virtual void clear();
	virtual void stats(unsigned &keys,unsigned &triggers);
	virtual void store(string const &key,set<string> const &triggers,time_t timeout,archive const &a);
	virtual ~process_cache();
	void *operator new(size_t n);
	void operator delete (void *p);
};


}

#endif
