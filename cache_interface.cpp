#define CPPCMS_SOURCE
#include "cache_interface.h"
#include "config.h"
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
	context_(context)
{
}


bool cache_interface::fetch_page(string const &key)
{
	if(nocache()) return false;

	bool zip = context_->response().need_gzip();

	std::string tmp;
	
	if(caching_module->fetch_page(key,tmp,zip)) {
		if(gzip)
			context_->response().context_encoding("gzip");
		context_->out().write(tmp.c_str(),tmp.size());
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
	bool zip = context_->response().need_gzip();
	cms->caching_module->store_page(key,context->response().copied_data(),triggers_,deadtime(timeout),zip)
}

void cache_interface::store_frame(std::string const &key,
				 std::string const &frame,
				 int timeout,
				 bool notriggers=false)
{
	store_frame(key,frame,set<std::string>(),timeout,notriggers);
}

void cache_interface::nocache()
{
	return caching_module_.get()!=0;
}

void cache_interface::has_cache()
{
	return !nocache();
}

void cache_interface::add_trigger(string const &t)
{
	if(nocache()) return false;
	triggers_.insert(t);
}

void cache_interface::rise(string const &t)
{
	if(nocache()) return;
	caching_module_->rise(t);
}

bool cache_interface::fetch_frame(string const &key,string &result,bool notriggers)
{
	return fetch(key,result,notriggers);
}

bool cache_interface::fetch(string const &key,string &result,bool notriggers)
{
	if(nocache()) return false;
	set<string> new_trig;
	if(caching_module_->fetch(key,result,new_trig)) {
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
	cache_module_->store(key,triggers,deadtime(timeout),data);
}

void cache_interface::clear()
{
	if(nocache()) return;
	caching_module_->clear();
}

bool cache_interface::stats(unsigned &k,unsigned &t)
{
	if(nocache()) return false;
	caching_module_->stats(k,t);
	return true;
}

void cache_interface::reset()
{
	triggers_.clear();
}


} // End of namespace cppcms
