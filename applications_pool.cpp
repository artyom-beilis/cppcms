#define CPPCMS_SOURCE
#include "applications_pool.h"
#include "application.h"
#include "service.h"
#include "global_config.h"
#include "cppcms_error.h"
#include <set>
#include <vector>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>

namespace cppcms {

	namespace {
		struct app_data : public util::noncopyable {
			app_data(std::string script,std::auto_ptr<applications_pool::factory> f) :
				script_name(script),
				match(0),
				use_regex(0),
				factory(f),
				size(0)
			{
				check();
			}
			app_data(std::string script,std::string pat,int select,std::auto_ptr<applications_pool::factory> f) :
				script_name(script),
				expr(pat),
				match(select),
				use_regex(1),
				factory(f),
				size(0)
			{
				check();
			}

			std::string script_name;
			boost::regex expr;
			int match;
			bool use_regex;
			std::auto_ptr<applications_pool::factory> factory;

			int size;
			std::set<application *> pool;

			~app_data()
			{
				std::set<application *>::iterator p;
				for(p=pool.begin();p!=pool.end();++p) {
					delete *p;
				}
			}

			void check() const
			{
				if(	!script_name.empty() 
					&& script_name[0]!='/' 
					&& script_name[0]!='.' 
					&& script_name!="*")
				{
					throw cppcms_error("Scipt name should be either '*', start with '.' or '/' or be empty");
				}
			}
		};

	}

	struct applications_pool::data {
		std::vector<boost::shared_ptr<app_data> > apps;
		int limit;
	};


applications_pool::applications_pool(service &srv,int limit) :
	srv_(&srv),
	d(new applications_pool::data())
{
	d->limit=limit;
}
applications_pool::~applications_pool()
{
}
void applications_pool::mount(std::auto_ptr<factory> aps)
{
	std::string script_name=srv_->settings().str("service.default_script_name","*");
	d->apps.push_back(boost::shared_ptr<app_data>(new app_data(script_name,aps)));
}
void applications_pool::mount(std::auto_ptr<factory> aps,std::string path_info,int select)
{
	std::string script_name=srv_->settings().str("service.default_script_name","*");
	d->apps.push_back(boost::shared_ptr<app_data>(new app_data(script_name,path_info,select,aps)));
}
void applications_pool::mount(std::auto_ptr<factory> aps,std::string script_name)
{
	d->apps.push_back(boost::shared_ptr<app_data>(new app_data(script_name,aps)));
}
void applications_pool::mount(std::auto_ptr<factory> aps,std::string script_name,std::string path_info,int select)
{
	d->apps.push_back(boost::shared_ptr<app_data>(new app_data(script_name,path_info,select,aps)));
}


std::auto_ptr<application> applications_pool::get(std::string script_name,std::string path_info,std::string &matched)
{
	for(unsigned i=0;i<d->apps.size();i++) {
		std::string const expected_name=d->apps[i]->script_name;
		if(expected_name!="*" && !expected_name.empty()) {
			if(expected_name[0]=='/')
				if(script_name!=expected_name)
					continue;
			else { // if(sn[0]=='.') 
				if(	script_name.size() <= expected_name.size() 
					|| script_name.substr(script_name.size() - expected_name.size())!=expected_name)
				{
					continue;
				}
			}
		}
		else if(expected_name=="*" && script_name.empty())
			continue;
		else if(expected_name.empty() && !script_name.empty())
			continue;
		
		boost::cmatch match;
		if(!d->apps[i]->use_regex) {
			matched=path_info;
		}
		else if(boost::regex_match(path_info.c_str(),match,d->apps[i]->expr)) {
			matched=match[d->apps[i]->match];
		}
		else {
			continue;
		}
		
		if(d->apps[i]->pool.empty()) {
			std::auto_ptr<application> app=(*d->apps[i]->factory)(*srv_);
			app->pool_id(i);
			return app;
		}
		d->apps[i]->size--;
		std::auto_ptr<application> app(*(d->apps[i]->pool.begin()));
		d->apps[i]->pool.erase(app.get());
		return app;
	}
	return std::auto_ptr<application>();
}

void applications_pool::put(std::auto_ptr<application> app)
{
	unsigned id=app->pool_id();
	if(id >= d->apps.size() || d->apps[id]->size >= d->limit)
		return;
	d->apps[id]->pool.insert(app.release());
	d->apps[id]->size++;
}


} //cppcms
