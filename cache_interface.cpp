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
#include "cache_interface.h"
#include "base_cache.h"
#include "cache_pool.h"
#include "http_context.h"
#include "http_response.h"
#include "json.h"
#include "service.h"
#include "cppcms_error.h"
#include <sstream>
#include <iostream>

namespace cppcms {
using namespace std;

namespace {
	const time_t infty=
		(sizeof(time_t)==4 ? 0x7FFFFFFF: 0x7FFFFFFFFFFFFFFFULL ) - 
			3600*24;
	time_t deadtime(int sec)
	{
		if(sec<0) 
			return infty;
		else {
			time_t tmp;
			time(&tmp);
			if(tmp+sec<tmp) {
				throw cppcms_error("Year 2038 problem?");
			}
			return tmp+sec;
		}
	}
}

struct cache_interface::data {};

cache_interface::cache_interface(http::context &context) :
	context_(&context),
	page_compression_used_(0)
{
	cache_module_ = context_->service().cache_pool().get();
}

cache_interface::~cache_interface()
{
}


bool cache_interface::fetch_page(string const &key)
{
	if(nocache()) return false;

	bool gzip = context_->response().need_gzip();
	page_compression_used_ = gzip;

	std::string r_key = (gzip ? "_Z:" : "_U:") + key;

	std::string tmp;

	if(cache_module_->fetch(r_key,tmp,0)) {
		if(gzip)
			context_->response().content_encoding("gzip");
		context_->response().out().write(tmp.c_str(),tmp.size());
		return true;
	}
	else {
		context_->response().copy_to_cache();	
		return false;
	}
}

void cache_interface::store_page(string const &key,int timeout)
{
	if(nocache()) return;

	context_->response().finalize();

	std::string r_key = (page_compression_used_ ? "_Z:" : "_U:") + key;
	triggers_.insert(key);
	cache_module_->store(r_key,context_->response().copied_data(),triggers_,deadtime(timeout));
}

void cache_interface::store_frame(std::string const &key,std::string const &frame,std::set<std::string> const &triggers,int timeout,bool notriggers)
{
	store(key,frame,triggers,timeout,notriggers);
}

void cache_interface::store_frame(std::string const &key,
				 std::string const &frame,
				 int timeout,
				 bool notriggers)
{
	store_frame(key,frame,set<std::string>(),timeout,notriggers);
}

bool cache_interface::nocache()
{
	return cache_module_.get()==0;
}

bool cache_interface::has_cache()
{
	return !nocache();
}

void cache_interface::add_trigger(string const &t)
{
	if(nocache()) return;
	triggers_.insert(t);
}

void cache_interface::rise(string const &t)
{
	if(nocache()) return;
	cache_module_->rise(t);
}

bool cache_interface::fetch_frame(string const &key,string &result,bool notriggers)
{
	return fetch(key,result,notriggers);
}

bool cache_interface::fetch(string const &key,string &result,bool notriggers)
{
	if(nocache()) return false;
	set<string> new_trig;

	if(cache_module_->fetch(key,result, (notriggers ? 0 : &new_trig))) {
		if(!notriggers)
			triggers_.insert(new_trig.begin(),new_trig.end());
		return true;
	}
	return false;
}

void cache_interface::store(string const &key,string const &data,
			set<string> const &triggers,
			int timeout,
			bool notriggers)
{
	if(nocache()) return;
	if(!notriggers) {
		this->triggers_.insert(triggers.begin(),triggers.end());
		this->triggers_.insert(key);
	}
	cache_module_->store(key,data,triggers,deadtime(timeout));
}

void cache_interface::clear()
{
	if(nocache()) return;
	cache_module_->clear();
}

bool cache_interface::stats(unsigned &k,unsigned &t)
{
	if(nocache()) return false;
	cache_module_->stats(k,t);
	return true;
}

void cache_interface::reset()
{
	triggers_.clear();
}


} // End of namespace cppcms
