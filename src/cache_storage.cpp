///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include <iostream>
#include <time.h>
#include <cppcms/cstdint.h>

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

	typedef std::map<
			string_type,
			container,
			std::less<string_type>,
			typename allocator::template rebind<std::pair<string_type,container> >::other
		> map_type;

	typedef typename map_type::iterator pointer;

	typedef std::list<
			pointer,
			typename allocator::template rebind<std::pair<string_type,container> >::other
		> lru_list_type;

	typedef std::multimap<
			string_type,
			pointer,
			std::less<string_type>,
			typename allocator::template rebind<std::pair<string_type,pointer> >::other
		> triggers_map_type;

	typedef std::list<
			typename triggers_map_type::iterator,
			typename allocator::template rebind<typename triggers_map_type::iterator>::other
		> triggers_list_type;

	typedef std::multimap<
			time_t,
			pointer,
			std::less<time_t>,
			typename allocator::template rebind<std::pair<time_t,pointer> >::other 
		> timeout_mmap_type;

	struct container {
		string_type data;
		typedef typename map_type::iterator pointer;
		typename lru_list_type::iterator lru;
		triggers_list_type triggers;
		typename timeout_mmap_type::iterator timeout;
		uint64_t generation;
	};

	map_type primary;
	triggers_map_type triggers;
	typedef typename triggers_map_type::iterator triggers_ptr;
	timeout_mmap_type timeout;
	typedef typename timeout_mmap_type::iterator timeout_ptr;
	lru_list_type lru;
	typedef typename lru_list_type::iterator lru_ptr;
	unsigned limit;
	size_t size;
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
			triggers.erase(*i);
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
	}
	~mem_cache()
	{
	}
	void set_size(unsigned l) { limit=l; };
	virtual bool fetch(std::string const &key,std::string *a,std::set<std::string> *triggers,time_t *timeout_out,uint64_t *gen)
	{
		rdlock_guard lock(*access_lock);
		pointer p;
		time_t now;
		time(&now);

		if((p=primary.find(to_int(key)))==primary.end() || p->second.timeout->first < now) {
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
				triggers->insert(to_std((*tp)->first));
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
		std::pair<triggers_ptr,triggers_ptr> range=triggers.equal_range(to_int(trigger));
		triggers_ptr p;
		std::list<pointer> kill_list;
		for(p=range.first;p!=range.second;p++) {
			kill_list.push_back(p->second);
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
		triggers.clear();
		size = 0;
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
		triggers=this->triggers.size();
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
		pointer p=primary.find(to_int(key));
		if(p==primary.end())
			return;
		delete_node(p);
	}

	virtual void store(	std::string const &key,
				std::string const &a,
				std::set<std::string> const &triggers_in,
				time_t timeout_in,
				uint64_t const *gen)
	{
		wrlock_guard lock(*access_lock);
		try {
			pointer main;
			main=primary.find(to_int(key));
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
			cont.data=to_int(a);
			if(gen)
				cont.generation=*gen;
			else
				cont.generation=generation++;
			lru.push_front(main);
			cont.lru=lru.begin();
			cont.timeout=timeout.insert(std::pair<time_t,pointer>(timeout_in,main));
			if(triggers_in.find(key)==triggers_in.end()){
				cont.triggers.push_back(triggers.insert(std::pair<string_type,pointer>(int_key,main)));
			}
			std::set<std::string>::const_iterator si;
			for(si=triggers_in.begin();si!=triggers_in.end();si++) {
				cont.triggers.push_back(triggers.insert(std::pair<string_type,pointer>(to_int(*si),main)));
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
booster::intrusive_ptr<base_cache> process_cache_factory(size_t memory)
{
	process_settings::init(memory);
	return new mem_cache<process_settings>(0);
}
#endif

} // impl 
} // cppcms

#endif // CPPCMS_NO_PREFOK_CACHE

