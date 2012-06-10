///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "session_memory_storage.h"
#include <cppcms/config.h>
#include "hash_map.h"
#include <booster/thread.h>
#include <time.h>
#include <map>

namespace cppcms {
namespace sessions { 

class session_memory_storage : public session_storage {
	struct _data;
	
	typedef cppcms::impl::hash_map<
		std::string,
		_data,
		cppcms::impl::string_hash<std::string>
		> map_type;
		
	typedef map_type::iterator pointer;
	typedef std::multimap<time_t,pointer> timeout_type;
	struct _data {
		time_t timeout;
		std::string info;
		timeout_type::iterator timeout_ptr;
	};
	
	map_type map_;
	timeout_type timeout_;
	booster::shared_mutex mutex_;
public:

	void save(std::string const &key,time_t to,std::string const &value)
	{
		booster::unique_lock<booster::shared_mutex> lock(mutex_);
		pointer p=map_.find(key);
		if(p==map_.end()) {
			std::pair<pointer,bool> pr = map_.insert(std::pair<std::string,_data>(key,_data()));
			pointer p = pr.first;
			p->second.timeout=to;
			p->second.info=value;
			p->second.timeout_ptr=timeout_.insert(std::pair<time_t,pointer>(to,p));
		}
		else {
			timeout_.erase(p->second.timeout_ptr);
			p->second.timeout=to;
			p->second.info=value;
			p->second.timeout_ptr=timeout_.insert(std::pair<time_t,pointer>(to,p));
		}
		short_gc();
	}


	void short_gc()
	{
		time_t now=::time(0);
		timeout_type::iterator p=timeout_.begin(),tmp;
		int count = 0;
		while(p!=timeout_.end() && p->first < now && count < 5) {
			tmp=p;
			++p;
			map_.erase(tmp->second);
			timeout_.erase(tmp);
			count++;
		}
	}

	bool load(std::string const &key,time_t &to,std::string &value)
	{
		booster::shared_lock<booster::shared_mutex> lock(mutex_);

		map_type::iterator p=map_.find(key);
		if(p==map_.end())
			return false;
		if(p->second.timeout < ::time(0))
			return false;
		value=p->second.info;
		to=p->second.timeout;
		return true;
	}

	void remove(std::string const &key) 
	{
		booster::unique_lock<booster::shared_mutex> lock(mutex_);

		pointer p=map_.find(key);
		if(p==map_.end())
			return;
		timeout_.erase(p->second.timeout_ptr);
		map_.erase(p);
		short_gc();
	}

	bool is_blocking()
	{
		return false;
	}
};

session_memory_storage_factory::session_memory_storage_factory() :
	storage_(new session_memory_storage())
{
}

session_memory_storage_factory::~session_memory_storage_factory()
{
}

booster::shared_ptr<session_storage> session_memory_storage_factory::get()
{
	return storage_;
}


bool session_memory_storage_factory::requires_gc()
{
	return false;
}

void session_memory_storage_factory::gc_job() 
{
}

} // sessions
} // cppcms
