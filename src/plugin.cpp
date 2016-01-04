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

struct single_entry {
	single_entry(manager::entry_point_type ep=NULL,char const *sig="") : entry(ep),signature(sig) {}
	manager::entry_point_type entry;
	std::string signature;
};

typedef std::map<std::string,single_entry> entries_type;
typedef std::map<std::string,entries_type> plugins_type;

struct manager::_data {
	plugins_type plugins;
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

void manager::add_entry(char const *plugin,char const *name,manager::entry_point_type e,char const *sig)
{
	try {
		booster::unique_lock<booster::mutex> guard(d->lock);
		d->plugins[plugin].insert(std::make_pair<std::string,single_entry>(name,single_entry(e,sig)));
	}
	catch(...){}
}
void manager::remove_entry(manager::entry_point_type e)
{
	try {
		booster::unique_lock<booster::mutex> guard(d->lock);
		for(plugins_type::iterator p=d->plugins.begin();p!=d->plugins.end();++p) {
			for(entries_type::iterator it = p->second.begin();it!=p->second.end();++it) {
				if(it->second.entry==e) {
					p->second.erase(it);
					if(p->second.empty())
						d->plugins.erase(p);
					return;
				}
			}
		}
	}
	catch(...){}
}

manager::entry_point_type manager::get_entry(std::string const &plugin,std::string const &name)
{
	booster::unique_lock<booster::mutex> guard(d->lock);
	plugins_type::const_iterator p=d->plugins.find(plugin);
	if(p==d->plugins.end())
		return 0;
	entries_type::const_iterator it=p->second.find(name);
	if(it!=p->second.end())
		return it->second.entry;
	return 0;
}

std::string manager::signature(std::string const &plugin,std::string const &name)
{
	booster::unique_lock<booster::mutex> guard(d->lock);
	plugins_type::const_iterator p=d->plugins.find(plugin);
	if(p==d->plugins.end())
		return "";
	entries_type::const_iterator it=p->second.find(name);
	if(it!=p->second.end())
		return it->second.signature;
	return "";
}
std::set<std::string> manager::plugins()
{
	std::set<std::string> r;
	booster::unique_lock<booster::mutex> guard(d->lock);
	for(plugins_type::const_iterator p=d->plugins.begin();p!=d->plugins.end();++p) {
		r.insert(p->first);
	}
	return r;
}

bool manager::has_plugin(std::string const &plugin)
{
	booster::unique_lock<booster::mutex> guard(d->lock);
	return d->plugins.find(plugin)!=d->plugins.end();
}

std::set<std::string> manager::entries(std::string const &name)
{
	std::set<std::string> r;
	booster::unique_lock<booster::mutex> guard(d->lock);
	plugins_type::const_iterator pit = d->plugins.find(name);
	if(pit==d->plugins.end())
		return r;
	for(entries_type::const_iterator p=pit->second.begin();p!=pit->second.end();++p) {
		r.insert(p->first);
	}
	return r;
}

signature_error::signature_error(std::string const &msg) : msg_(msg) {}
signature_error::~signature_error() throw() {}
char const *signature_error::what() const throw()
{
	return msg_.c_str();
}

} // plugin
} // cppcms
