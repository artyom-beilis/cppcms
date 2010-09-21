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
#include "session_memory_storage.h"
#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/unordered_map.hpp>
#else // Internal Boost
#   include <cppcms_boost/unordered_map.hpp>
    namespace boost = cppcms_boost;
#endif
#include <booster/thread.h>
#include <time.h>
#include <map>

namespace cppcms {
namespace sessions { 

class session_memory_storage : public session_storage {
	typedef std::multimap<time_t,std::string> timeout_type;
	struct _data {
		time_t timeout;
		std::string info;
		timeout_type::iterator timeout_ptr;
	};
	typedef boost::unordered_map<std::string,_data> map_type;
	map_type map_;
	timeout_type timeout_;
	booster::shared_mutex mutex_;
public:

	void save(std::string const &key,time_t to,std::string const &value)
	{
		booster::unique_lock<booster::shared_mutex> lock(mutex_);
		map_type::iterator p=map_.find(key);
		if(p==map_.end()) {
			_data &d=map_[key];
			d.timeout=to;
			d.info=value;
			d.timeout_ptr=timeout_.insert(std::pair<time_t,std::string>(to,key));
		}
		else {
			timeout_.erase(p->second.timeout_ptr);
			p->second.timeout=to;
			p->second.info=value;
			p->second.timeout_ptr=timeout_.insert(std::pair<time_t,std::string>(to,key));
		}
		gc();
	}


	void gc()
	{
		time_t now=::time(0);
		timeout_type::iterator p=timeout_.begin(),tmp;
		while(p!=timeout_.end() && p->first < now) {
			tmp=p;
			++p;
			map_.erase(tmp->second);
			timeout_.erase(tmp);
		}
	}

	bool load(std::string const &key,time_t &to,std::string &value)
	{
		booster::shared_lock<booster::shared_mutex> lock(mutex_);

		map_type::const_iterator p=map_.find(key);
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

		map_type::iterator p=map_.find(key);
		if(p==map_.end())
			return;
		timeout_.erase(p->second.timeout_ptr);
		map_.erase(p);
		gc();
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
