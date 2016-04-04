///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2016  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/plugin.h>
#include <cppcms/json.h>
#include <cppcms/service.h>
#include <map>
#include <booster/locale/format.h>
#include <booster/thread.h>
#include <booster/shared_ptr.h>
#include <booster/shared_object.h>

namespace cppcms {
namespace plugin {

namespace {
	struct init {
		init() { 
			manager::instance(); 
			scope::is_loaded("");
		}
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


struct scope::_class_data {
	booster::mutex lock;
	std::set<std::string> modules;
};

scope::_class_data &scope::class_data()
{
	static _class_data d;
	return d;
}

struct scope::_data {
	std::vector<std::string> paths;
	std::string pattern;
	std::map<std::string,booster::shared_ptr<booster::shared_object> > objects;
};

scope::scope() : d(new scope::_data())
{
}

scope::scope(json::value const &v) : d(new scope::_data())
{
	init(v);
}

scope::~scope() 
{
	try {
		_class_data &cls = class_data();
		booster::unique_lock<booster::mutex> guard(cls.lock);
		for(std::map<std::string,booster::shared_ptr<booster::shared_object> >::iterator p=d->objects.begin();p!=d->objects.end();++p) {
			cls.modules.erase(p->first);
		}
		d->objects.clear();
	}
	catch(...) {}
}

scope::scope(int argc,char **argv) : d(new scope::_data())
{
	json::value v = service::load_settings(argc,argv);
	init(v);
}

void scope::init(json::value const &v)
{
	d->paths = v.get("plugin.paths",std::vector<std::string>());
	d->pattern = v.get("plugin.shared_object_pattern",std::string());
	
	std::vector<std::string> modules = v.get("plugin.modules",std::vector<std::string>());
	for(size_t i=0;i<modules.size();i++) {
		load(modules[i]);
	}

}

bool scope::is_loaded(std::string const &name)
{
	_class_data &cls = class_data();
	booster::unique_lock<booster::mutex> guard(cls.lock);
	return cls.modules.find(name)!=cls.modules.end();
}

void scope::paths(std::vector<std::string> const &paths)
{
	d->paths = paths;
}
void scope::shared_object_pattern(std::string const &p)
{
	d->pattern = p;
}

void scope::load(std::string const &name)
{
	_class_data &cls = class_data();
	booster::unique_lock<booster::mutex> guard(cls.lock);
	if(cls.modules.find(name)!=cls.modules.end())
		return;
	std::string so_name;
	if(d->pattern.empty())
		so_name = booster::shared_object::name(name);
	else
		so_name = (booster::locale::format(d->pattern) % name).str(std::locale::classic());

	booster::shared_ptr<booster::shared_object> obj(new booster::shared_object());
	int dlflags = booster::shared_object::load_lazy |  booster::shared_object::load_global;
	if(d->paths.empty()) {
		if(!obj->open(so_name,dlflags)) 
			throw cppcms_error("Failed to load " + so_name);
	}
	else {
		for(size_t i=0;i<d->paths.size();i++) {
			std::string path  = d->paths[i];
			if(path.empty())
				path = so_name;
			else
				path = path + "/" + so_name;
			if(obj->open(path,dlflags))
				break;
		}
		if(!obj->is_open()) {
			std::ostringstream ss;
			ss << "Failed to load " << so_name << " from ";
			for(size_t i=0;i<d->paths.size();i++) {
				if(i!=0) {
					ss << ", ";
				}
				ss << "`" << d->paths[i] << "'";
			}
			throw cppcms_error(ss.str());
		}
	}
	d->objects[name]=obj;
	cls.modules.insert(name);
}

bool scope::is_loaded_by_this_scope(std::string const &module) const
{
	return d->objects.find(module)!=d->objects.end();
}

booster::shared_object const &scope::get(std::string const &module) const
{
	std::map<std::string,booster::shared_ptr<booster::shared_object> >::const_iterator p = d->objects.find(module);
	if(p==d->objects.end())
		throw cppcms_error("Module `" + module + "' wasn't loaded withing this scope");
	return *p->second;
}


} // plugin
} // cppcms
