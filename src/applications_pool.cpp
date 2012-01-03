///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/applications_pool.h>
#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/mount_point.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/json.h>
#include <set>
#include <vector>
#include <cppcms/config.h>
#include <booster/regex.h>
#include <booster/shared_ptr.h>
#include <booster/thread.h>

namespace cppcms {

	struct applications_pool::basic_app_data : public booster::noncopyable {
		basic_app_data(mount_point const &p) :
			mount_point_(p)
		{
		}
		mount_point mount_point_;
	};
	
	struct applications_pool::app_data : public applications_pool::basic_app_data {
		app_data(mount_point const &p,std::auto_ptr<applications_pool::factory> f) :
			basic_app_data(p),
			factory(f),
			size(0)
		{
		}
		
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

	};

	struct applications_pool::long_running_app_data : public applications_pool::basic_app_data
	{
		long_running_app_data(mount_point const &p) : basic_app_data(p) 
		{
		}
	};

	struct applications_pool::_data {
		std::vector<booster::shared_ptr<app_data> > apps;
		typedef std::map<application *,booster::shared_ptr<long_running_app_data> > long_running_aps_type;
		long_running_aps_type long_running_aps;
		int limit;
		booster::recursive_mutex mutex;
	};
	typedef booster::unique_lock<booster::recursive_mutex> lock_it;


applications_pool::applications_pool(service &srv,int limit) :
	srv_(&srv),
	d(new applications_pool::_data())
{
	d->limit=limit;
}
applications_pool::~applications_pool()
{
}

void applications_pool::mount(std::auto_ptr<factory> aps)
{
	lock_it lock(d->mutex);
	d->apps.push_back(booster::shared_ptr<app_data>(new app_data(mount_point(),aps)));
}
void applications_pool::mount(std::auto_ptr<factory> aps,mount_point const &p)
{
	lock_it lock(d->mutex);
	d->apps.push_back(booster::shared_ptr<app_data>(new app_data(p,aps)));
}
void applications_pool::mount(booster::intrusive_ptr<application> app)
{
	lock_it lock(d->mutex);
	d->long_running_aps[app.get()]=
		booster::shared_ptr<long_running_app_data>(new long_running_app_data(mount_point()));
}
void applications_pool::mount(booster::intrusive_ptr<application> app,mount_point const &p)
{
	lock_it lock(d->mutex);
	d->long_running_aps[app.get()]=
		booster::shared_ptr<long_running_app_data>(new long_running_app_data(p));
}

booster::intrusive_ptr<application> applications_pool::get(	char const *host,
								char const *script_name,
								char const *path_info,
								std::string &m)
{
	lock_it lock(d->mutex);
	for(unsigned i=0;i<d->apps.size();i++) {
		std::pair<bool,std::string> match = d->apps[i]->mount_point_.match(host,script_name,path_info);
		if(match.first==false)
			continue;
		m=match.second;

		if(d->apps[i]->pool.empty()) {
			booster::intrusive_ptr<application> app=(*d->apps[i]->factory)(*srv_).release();
			app->pool_id(i);
			return app;
		}

		d->apps[i]->size--;
		booster::intrusive_ptr<application> app(*(d->apps[i]->pool.begin()));
		d->apps[i]->pool.erase(app.get());
		return app;
	}
	for(_data::long_running_aps_type::iterator p=d->long_running_aps.begin();p!=d->long_running_aps.end();++p){
		std::pair<bool,std::string> match = p->second->mount_point_.match(host,script_name,path_info);
		if(match.first == false)
			continue;
		m=match.second;
		booster::intrusive_ptr<application> app=p->first;
		return app;
	}
	return 0;
}

void applications_pool::put(application *app)
{
	lock_it lock(d->mutex);
	if(!app) return;
	int id=app->pool_id();
	if(id < 0) {
		d->long_running_aps.erase(app);
		delete app;
		return;
	}
	if(unsigned(id) >= d->apps.size() || d->apps[id]->size >= d->limit) {
		delete app;
		return;
	}
	d->apps[id]->pool.insert(app);
	d->apps[id]->size++;
}


} //cppcms
