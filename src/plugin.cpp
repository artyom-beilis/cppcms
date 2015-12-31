///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2016  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/plugin.h>
#include <map>
#include <booster/thread.h>

namespace cppcms {
namespace plugin {

namespace {
	struct init {
		init() { manager::instance(); }
	} init_inst;
}

struct manager::_data {
	std::map<std::string,manager::entry_point_type> mapping;
	booster::mutex lock;
};


manager &manager::instance()
{
	static manager m;
	return m;
}

manager::manager() : d(new manager::_data()) 
{
}
manager::~manager()
{
}

void manager::add_entry(char const *name,manager::entry_point_type e)
{
	try {
		booster::unique_lock<booster::mutex> guard(d->lock);
		std::string key=name;
		d->mapping.insert(std::make_pair(key,e));
	}
	catch(...){}
}
void manager::remove_entry(manager::entry_point_type e)
{
	try {
		booster::unique_lock<booster::mutex> guard(d->lock);
		for(std::map<std::string,manager::entry_point_type>::iterator p=d->mapping.begin();p!=d->mapping.end();++p) {
			if(p->second==e) {
				d->mapping.erase(p);
				return;
			}
		}
	}
	catch(...){}
}

manager::entry_point_type manager::get_entry(std::string const &name)
{
	booster::unique_lock<booster::mutex> guard(d->lock);
	std::map<std::string,manager::entry_point_type>::const_iterator p=d->mapping.find(name);
	if(p==d->mapping.end())
		return 0;
	return p->second;
	
}


} // plugin
} // cppcms
