///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/config.h>
#ifndef CPPCMS_NO_CACHE
#include "cache_storage.h"
#include <booster/thread.h>



#if defined CPPCMS_WIN32 
# ifndef CPPCMS_NO_PREFOK_CACHE
#   define CPPCMS_NO_PREFOK_CACHE
# endif
#endif

#ifndef CPPCMS_NO_PREFOK_CACHE
# include "posix_util.h"
# include "shmem_allocator.h"
#endif

#include <map>
#include <list>
#include <limits>
#include <memory>
#include <time.h>
#include <cppcms/cstdint.h>


#include "hash_map.h"

namespace cppcms {
namespace impl {

template<typename String>
struct copy_traits {
	static String to_int(std::string const &other)
	{
		return String(other.c_str(),other.size());
	}
	static std::string to_std(String const &other)
	{
		return std::string(other.c_str(),other.size());
	}
};

template<>
struct copy_traits<std::string>
{
	static std::string to_int(std::string const &a) { return a; }
	static std::string to_std(std::string const &a) { return a; }
};

struct string_equal {
	template<typename S1,typename S2>
	bool operator()(S1 const &s1,S2 const &s2) const
	{
		return s1.size() == s2.size() && memcmp(s1.c_str(),s2.c_str(),s1.size()) == 0;
	}
};

#ifndef CPPCMS_NO_PREFOK_CACHE

struct process_settings {
	static shmem_control *process_memory;

	typedef ::cppcms::impl::mutex mutex_type;
	typedef ::cppcms::impl::shared_mutex shared_mutex_type;
	typedef mutex_type::guard lock_guard;
	typedef shared_mutex_type::shared_guard rdlock_guard;
	typedef shared_mutex_type::unique_guard wrlock_guard;
	typedef shmem_allocator<char,process_memory> allocator;
	
	static bool not_enough_memory()
	{
		size_t mem_size = process_memory->size() / 10;
		void *p=process_memory->malloc(mem_size);
		if(!p)  { 
			return true;
		}
		process_memory->free(p);
		return false;
	}
	
	static size_t size_limit()
	{
		return process_memory->size() / 20;
	}
	
	
	static void init(size_t size)
	{
		if(process_memory)
			return;
		process_memory=new shmem_control(size);
	}
	static const bool process_shared = true;
};

shmem_control *process_settings::process_memory=0;

#endif

struct thread_settings {
	typedef booster::mutex mutex_type;
	typedef booster::shared_mutex shared_mutex_type;
	typedef booster::unique_lock<booster::mutex> lock_guard;
	typedef booster::shared_lock<booster::shared_mutex> rdlock_guard;
	typedef booster::unique_lock<booster::shared_mutex> wrlock_guard;
	typedef std::allocator<char> allocator;
	static bool not_enough_memory() { return false; }
	static size_t size_limit() { return std::numeric_limits<size_t>::max(); }
	static const bool process_shared = false;
};


template<typename Setup>
class mem_cache : public base_cache {
	typedef typename Setup::mutex_type mutex_type;
	typedef typename Setup::shared_mutex_type shared_mutex_type;
	std::auto_ptr<mutex_type> lru_mutex;
	std::auto_ptr<shared_mutex_type> access_lock;
	typedef typename Setup::allocator allocator;
	typedef typename Setup::lock_guard lock_guard;
	typedef typename Setup::rdlock_guard rdlock_guard;
	typedef typename Setup::wrlock_guard wrlock_guard;
	typedef typename allocator::template rebind<mem_cache>::other this_allocator;

	bool not_enough_memory()
	{
		return Setup::not_enough_memory();
	}

	size_t size_limit()
	{
		return Setup::size_limit();
	}

	struct container;

	typedef std::basic_string<char,std::char_traits<char>,allocator > string_type;

	typedef hash_map<
			string_type,
			container,
			string_hash,
			string_equal,
			typename allocator::template rebind<std::pair<const string_type,container> >::other
		> map_type;

	typedef typename map_type::iterator pointer;

	typedef std::list<
			pointer,
			typename allocator::template rebind<pointer>::other
		> pointer_list_type;

	typedef hash_map<
			string_type,
			pointer_list_type,
			string_hash,
			string_equal,
			typename allocator::template rebind<std::pair<const string_type,pointer_list_type> >::other
		> triggers_map_type;

	typedef std::pair<
			typename triggers_map_type::iterator,
			typename pointer_list_type::iterator
		> trigger_ptr_type;

	typedef std::list<
			trigger_ptr_type,
			typename allocator::template rebind<trigger_ptr_type>::other
		> triggers_list_type;

	typedef std::multimap<
			time_t,
			pointer,
			std::less<time_t>,
			typename allocator::template rebind<std::pair<const time_t,pointer> >::other 
		> timeout_mmap_type;

	struct container {
		string_type data;
		typedef typename map_type::iterator pointer;
		typename pointer_list_type::iterator lru;
		triggers_list_type triggers;
		typename timeout_mmap_type::iterator timeout;
		uint64_t generation;
	};

	map_type primary;
	triggers_map_type triggers;
	typedef typename triggers_map_type::iterator triggers_ptr;
	timeout_mmap_type timeout;
	typedef typename timeout_mmap_type::iterator timeout_ptr;
	pointer_list_type lru;
	typedef typename pointer_list_type::iterator lru_ptr;
	unsigned limit;
	size_t size;
	size_t triggers_count;
	int refs;
	uint64_t generation;

	string_type to_int(std::string const &other)
	{
		return copy_traits<string_type>::to_int(other);
	}
	std::string to_std(string_type const &other)
	{
		return copy_traits<string_type>::to_std(other);
	}

	void delete_node(pointer p)
	{
		lru.erase(p->second.lru);
		timeout.erase(p->second.timeout);
		typename triggers_list_type::iterator i;
		for(i=p->second.triggers.begin();i!=p->second.triggers.end();i++) {
			i->first->second.erase(i->second);
			triggers_count --;
			if(i->first->second.empty())
				triggers.erase(i->first);
		}
		primary.erase(p);
		size--;
	}

public:
	mem_cache(unsigned pages=0) : 
		lru_mutex(new mutex_type()),
		access_lock(new shared_mutex_type()),
		limit(pages),
		size(0),
		refs(0),
		generation(0)
	{
		nl_clear();
	}
	~mem_cache()
	{
	}
	void set_size(unsigned l) 
	{ 
		limit = l; 
		nl_clear();
	};
	virtual bool fetch(std::string const &key,std::string *a,std::set<std::string> *triggers,time_t *timeout_out,uint64_t *gen)
	{
		rdlock_guard lock(*access_lock);
		pointer p;
		time_t now;
		time(&now);

		if((p=primary.find(key))==primary.end() || p->second.timeout->first < now) {
			return false;
		}

		{ // Update LRU
			lock_guard lock(*lru_mutex);
			lru.erase(p->second.lru);
			lru.push_front(p);
			p->second.lru=lru.begin();
		}

		if(a)
			*a=to_std(p->second.data);

		if(triggers) {
			typename triggers_list_type::iterator tp;
			for(tp=p->second.triggers.begin();tp!=p->second.triggers.end();tp++) {
				triggers->insert(to_std(tp->first->first));
			}
		}

		if(timeout_out) {
			*timeout_out=p->second.timeout->first;
		}		

		if(gen)
			*gen=p->second.generation;

		return true;
	}
	virtual void rise(std::string const &trigger)
	{
		wrlock_guard lock(*access_lock);
		triggers_ptr p = triggers.find(trigger);
		if(p==triggers.end())
			return;
		std::list<pointer> kill_list;
		for(typename pointer_list_type::iterator it=p->second.begin();it!=p->second.end();++it) {
			kill_list.push_back(*it);
		}
		typename std::list<pointer>::iterator lptr;

		for(lptr=kill_list.begin();lptr!=kill_list.end();lptr++) {
			delete_node(*lptr);
		}
	}
	void nl_clear()
	{
		timeout.clear();
		lru.clear();
		primary.clear();
		primary.rehash(limit);
		triggers.clear();
		triggers.rehash(limit);
		size = 0;
		triggers_count = 0;
	}
	virtual void clear()
	{
		wrlock_guard lock(*access_lock);
		nl_clear();
	}
	virtual void stats(unsigned &keys,unsigned &triggers)
	{
		rdlock_guard lock(*access_lock);
		keys=size;
		triggers = triggers_count;
	}
	void check_limits()
	{
		pointer main=primary.end();
		time_t now;
		time(&now);

		while(size > 0 && (not_enough_memory() || (size>=limit && limit>0)))
		{
			if(!timeout.empty() && timeout.begin()->first<now) {
				main=timeout.begin()->second;
			}
			else if(!lru.empty()){
				main=*lru.rbegin();
			}
			else
				break;
			delete_node(main);
		}
	}
	virtual void remove(std::string const &key)
	{
		wrlock_guard lock(*access_lock);
		pointer p=primary.find(key);
		if(p==primary.end())
			return;
		delete_node(p);
	}

	void add_trigger(pointer p,std::string const &key)
	{
		std::pair<string_type,pointer_list_type> tr(to_int(key),pointer_list_type());
		std::pair<triggers_ptr,bool> r=triggers.insert(tr);
		triggers_ptr it = r.first;
		it->second.push_front(p);
		p->second.triggers.push_back(trigger_ptr_type(it,it->second.begin()));
		triggers_count++;
	}

	virtual void store(	std::string const &key,
				std::string const &a,
				std::set<std::string> const &triggers_in,
				time_t timeout_in,
				uint64_t const *gen)
	{
		string_type ar;
		try {
			string_type tmp = to_int(a);
			ar.swap(tmp);
		}
		catch(std::bad_alloc const &) {
			return;
		}

		wrlock_guard lock(*access_lock);
		try {
			pointer main;
			main=primary.find(key);
			if(main!=primary.end())
				delete_node(main);
			if(size > size_limit())
				return;
			check_limits();
			string_type int_key = to_int(key);

			std::pair<pointer,bool> res=primary.insert(std::pair<string_type,container>(int_key,container()));
			size ++;
			main=res.first;
			container &cont=main->second;
			cont.data.swap(ar);
			if(gen)
				cont.generation=*gen;
			else
				cont.generation=generation++;
			lru.push_front(main);
			cont.lru=lru.begin();
			cont.timeout=timeout.insert(std::pair<time_t,pointer>(timeout_in,main));
			if(triggers_in.find(key)==triggers_in.end()){
				add_trigger(main,key);
			}
			std::set<std::string>::const_iterator si;
			for(si=triggers_in.begin();si!=triggers_in.end();si++) {
				add_trigger(main,*si);
			}
		}
		catch(std::bad_alloc const &e)
		{
			nl_clear();
		}
	}
	virtual void add_ref()
	{
		if(Setup::process_shared)
			return;
		wrlock_guard lock(*access_lock);
		refs++;
	}
	virtual bool del_ref()
	{
		// This object should not be deleted because it is created only once
		// and exists in memory
		if(Setup::process_shared)
			return false;
		wrlock_guard lock(*access_lock);
		refs--;
		if(refs==0)
			return true;
		return false;
	}

	void *operator new(size_t /*n*/) 
	{
		return this_allocator().allocate(1);
	}

	void operator delete(void *p)
	{
		this_allocator().deallocate(reinterpret_cast<mem_cache *>(p),1);
	}

}; // mem cache


booster::intrusive_ptr<base_cache> thread_cache_factory(unsigned items)
{
	return new mem_cache<thread_settings>(items);
}

#if !defined(CPPCMS_NO_PREFOK_CACHE)
booster::intrusive_ptr<base_cache> process_cache_factory(size_t memory,unsigned items)
{
	process_settings::init(memory);
	return new mem_cache<process_settings>(items);
}
#endif

} // impl 
} // cppcms

#endif // CPPCMS_NO_PREFOK_CACHE

