#ifndef CPPCMS_PROC_CHACHE_H
#define CPPCMS_PROC_CHACHE_H
#include "base_cache.h"
#include "cache_interface.h"
#include "pthread.h"
#include <map>
#include <list>
namespace cppcms {

namespace memory {
extern shmem_control *mem;
};

using namespace std;

class process_cache : public base_cache {
	pthread_mutex_t lru_mutex;
	pthread_rwlock_t access_lock;
	typedef std::basic_string<char,std::char_traits<char>,shmem_allocator<char,mem> > shr_string;
	struct container {
		shr_string data;
		typedef map<shr_string,container,std::less<shr_string>,
			shmem_allocator<std::pair<shr_string,container>,mem > > primary_map;
		typedef primary_map::iterator pointer;
		typedef list<pointer,shmem_allocator<pointer,mem> > pointer_list;
		typedef pointer_list::iterator lru;
		typedef multimap<shr_string,pointer,std::less<shr_string>,
				shmem_allocator<std::pair<shr_string,pointer>,mem > > secondary_map;

		list<secondary_map::iterator,shmem_allocator<secondary_map::iterator,mem> > triggers;
		typedef multimap<time_t,pointer,std::less<time_t>,
				shmem_allocator<std::pair<time_t,pointer>,mem > > timeout_map;
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
	unsigned limit;

	string *get(string const &key,set<string> *triggers);
	void delete_node(pointer p);
	void print_all();
	bool debug_mode;
	int fd;

public:
	process_cache(unsigned pages=0) :
		 limit(pages) 
	{
		pthread_mutexattr_t a;
		pthread_mutexattr_init(&a);
		pthread_mutexattr_setpshared(&a,PTHREAD_PROCESS_SHARED);

		pthread_mutex_init(&lru_mutex,&a);
		pthread_mutexattr_destroy(&a);


		pthread_mutexrwlock_t al;
		pthread_mutexrwlock_init(&al);
		pthread_mutexrwlock_setpshared(&al,PTHREAD_PROCESS_SHARED);
		pthread_rwlock_init(&access_lock,&al);
		pthread_mutexwrlock_destroy(&al);
		debug_mode=false;
	};
	void set_size(unsigned l) { limit=l; };
	virtual bool fetch_page(string const &key,string &output,bool gzip);
	virtual bool fetch(string const &key,archive &a,set<string> &tags);
	virtual void rise(string const &trigger);
	virtual void clear();
	virtual void stats(unsigned &keys,unsigned &triggers);
	virtual void store(string const &key,set<string> const &triggers,time_t timeout,archive const &a);
	virtual ~process_cache();
};

class process_cache_factory : public cache_factory{
	process_cache *cache;
	shmem_control *memory;
public:
	thread_cache_factory(unsigned n) : cache(new process_cache(n)) {};
	virtual base_cache *get() const { return cache; };
	virtual void del(base_cache *p) const { };
	virtual ~thread_cache_factory() { delete cache; };
};

}

#endif
