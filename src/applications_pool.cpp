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
#include <list>
#include <vector>
#include <cppcms/config.h>
#include <booster/regex.h>
#include <booster/shared_ptr.h>
#include <booster/thread.h>
#include <booster/log.h>


namespace cppcms {

class application_specific_pool::_policy {
public:
	_policy(application_specific_pool *self) : self_(self) {}
	virtual void prepopulate(cppcms::service &srv) = 0;
	virtual ~_policy() {}
	virtual booster::intrusive_ptr<application> get(cppcms::service &srv) = 0;
	virtual void put(application *app) = 0;
	application *get_new(cppcms::service &srv) { return self_->get_new(srv); }
protected:
	application_specific_pool *self_;
};

class application_specific_pool::_tls_policy : public application_specific_pool::_policy {
public:
	_tls_policy(application_specific_pool *self) : application_specific_pool::_policy(self) {}

	virtual booster::intrusive_ptr<application> get(cppcms::service &srv)
	{
		application *app = tss_.release();
		if(!app)
			return get_new(srv);
		return app;
	}
	virtual void put(application *app)
	{
		if(!app)
			return;
		tss_.reset(app);
	}
	virtual void prepopulate(cppcms::service &){}
private:
	booster::thread_specific_ptr<application> tss_;
};

class application_specific_pool::_pool_policy : public application_specific_pool::_policy{
public:
	_pool_policy(application_specific_pool *self,size_t n) : 
		_policy(self)
	{
		apps_.resize(n,0);
		size_ = 0;
	}
	~_pool_policy()
	{
		for(size_t i=0;i<size_;i++)
			delete apps_[i];
	}
	virtual void prepopulate(cppcms::service &srv)
	{
		if((self_->flags() & app::prepopulated) && !(self_->flags() & app::legacy)) {
			while(size_ < apps_.size()) {
				size_++;
				apps_[size_-1]= get_new(srv);
			}
		}
	}
	virtual booster::intrusive_ptr<application> get(cppcms::service &srv)
	{
		if(size_ == 0)
			return get_new(srv);
		size_ --;
		application *app = apps_[size_];
		apps_[size_]=0;
		return app;
	}
	virtual void put(application *app)
	{
		if(!app)
			return;
		if(size_ >= apps_.size())
			delete app;
		apps_[size_++] = app;
	}
private:
	std::vector<application *> apps_;
	size_t size_;
};

class application_specific_pool::_async_policy : public application_specific_pool::_policy{
public:
	_async_policy(application_specific_pool *self) : 
		_policy(self) 
	{
	}
	virtual void prepopulate(cppcms::service &srv)
	{
		if((self_->flags() & app::prepopulated) && !(self_->flags() & app::legacy)) {
			if(!app_)
				app_ = get_new(srv);
		}
	}
	virtual booster::intrusive_ptr<application> get(cppcms::service &srv)
	{
		if(!app_)
			app_ = get_new(srv);
		return app_;
	}
	virtual void put(application *)
	{
		// SHOULD NEVER BE CALLED as when pool is destroyed and app_ is destroyed weak_ptr would be invalid
	}
private:
	booster::intrusive_ptr<application> app_;
};

class application_specific_pool::_async_legacy_policy : public application_specific_pool::_policy{
public:
	_async_legacy_policy(application_specific_pool *self) : 
		_policy(self),
		app_(0)
	{
	}
	virtual void prepopulate(cppcms::service &) {}
	virtual booster::intrusive_ptr<application> get(cppcms::service &srv)
	{
		if(self_->flags()==-1)
			return 0;
		if(!app_)
			app_ = get_new(srv);
		return app_;
	}
	virtual void put(application *app)
	{
		if(!app)
			return;
		delete app;
		app_ = 0;
		self_->flags(-1);
	}
private:
	application *app_;
};


struct application_specific_pool::_data {
	int flags;
	size_t size;
	booster::hold_ptr<application_specific_pool::_policy> policy;
	booster::recursive_mutex lock;
};

application_specific_pool::~application_specific_pool()
{
}

application_specific_pool::application_specific_pool() : d(new application_specific_pool::_data()) 
{
	d->flags = 0;
	d->size = 0;
}

int application_specific_pool::flags()
{
	return d->flags;
}

void application_specific_pool::flags(int flags)
{
	if(d->flags == -1 || d->policy.get())
		return;
	d->flags = flags;
	if(flags == -1) 
		return;	

	if(flags == (app::asynchronous | app::legacy)) {
		d->policy.reset(new _async_legacy_policy(this));
		return;
	}

	if(flags == app::legacy) {
		d->policy.reset(new _pool_policy(this,d->size));
		return;
	}

	if((flags & app::op_mode_mask) != app::synchronous) {
		d->policy.reset(new _async_policy(this));
		return;
	}
	if(flags & app::thread_specific) {
		d->policy.reset(new _tls_policy(this));
	}
	else {
		d->policy.reset(new _pool_policy(this,d->size));
	}
}

void application_specific_pool::size(size_t n)
{
	d->size = n;
}

application *application_specific_pool::get_new(service &srv)
{
	application *a = new_application(srv);
	if(!a)
		return 0;
	a->set_pool(shared_from_this());
	return a;
}

void application_specific_pool::put(application *a)
{
	booster::unique_lock<booster::recursive_mutex> g(d->lock);

	if(d->flags == -1) {
		delete a;
		return;
	}
	assert(d->policy.get());
	d->policy->put(a);
}

booster::intrusive_ptr<application> application_specific_pool::get(cppcms::service &srv)
{
	booster::unique_lock<booster::recursive_mutex> g(d->lock);

	if(d->flags == -1)
		return 0;
	assert(d->policy.get());
	booster::intrusive_ptr<application> app = d->policy->get(srv);
	return app;
}

void application_specific_pool::prepopulate(cppcms::service &srv)
{
	d->policy->prepopulate(srv);
}

namespace impl {
	class legacy_sync_pool : public application_specific_pool {
	public:
		legacy_sync_pool(std::auto_ptr<applications_pool::factory> f)
		{
			fact_ = f;
		}
		application *new_application(cppcms::service &srv)
		{
			std::auto_ptr<application> a = (*fact_)(srv);
			return a.release();
		}
	private:
		std::auto_ptr<applications_pool::factory> fact_;	
	};

	class legacy_async_pool : public application_specific_pool
	{
	public:
		legacy_async_pool(booster::intrusive_ptr<application> app)
		{
			my_ = app.get();
		}
		application *new_application(cppcms::service &) 
		{
			application *a = my_;
			my_ = 0;
			return a;
		}
	private:
		application *my_;
	};
} // impl

struct applications_pool::_data {

	struct attachment {
		mount_point mp;
		booster::shared_ptr<application_specific_pool> pool;
		attachment(booster::shared_ptr<application_specific_pool> p,mount_point const &m) : mp(m), pool(p) {}
	};
	std::list<attachment> apps;
	std::list<attachment> legacy_async_apps;
	int legacy_limit;
	int thread_count;
	booster::recursive_mutex lock;
};


applications_pool::applications_pool(service &srv,int pool_size_limit) :
	srv_(&srv),
	d(new applications_pool::_data())
{
	d->legacy_limit=pool_size_limit;
	d->thread_count = srv_->threads_no();
}
applications_pool::~applications_pool()
{
}


void applications_pool::mount(std::auto_ptr<factory> aps,mount_point const &mp)
{
	booster::shared_ptr<application_specific_pool> p(new impl::legacy_sync_pool(aps));
	p->size(d->legacy_limit);
	p->flags(app::legacy);

	booster::unique_lock<booster::recursive_mutex> lock(d->lock);
	d->apps.push_back(_data::attachment(p,mp));
}

void applications_pool::mount(std::auto_ptr<factory> aps)
{
	mount(aps,mount_point());
}
void applications_pool::mount(booster::intrusive_ptr<application> app)
{
	mount(app,mount_point());
}
void applications_pool::mount(booster::intrusive_ptr<application> app,mount_point const &mp)
{
	booster::shared_ptr<application_specific_pool> p(new impl::legacy_async_pool(app));
	p->flags(app::legacy | app::asynchronous);

	booster::unique_lock<booster::recursive_mutex> lock(d->lock);
	d->legacy_async_apps.push_back(_data::attachment(p,mp));
}

void applications_pool::mount(booster::shared_ptr<application_specific_pool> gen,mount_point const &point,int flags)
{
	if(flags & app::legacy) {
		throw cppcms_error("Direct specification of cppcms::app::legacy flag is forbidden");
	}
	gen->size(d->thread_count);
	gen->flags(flags);
	if(flags & app::prepopulated)
		gen->prepopulate(*srv_);
	booster::unique_lock<booster::recursive_mutex> lock(d->lock);
	d->apps.push_back(_data::attachment(gen,point));
}

void applications_pool::mount(booster::shared_ptr<application_specific_pool> gen,int flags)
{
	mount(gen,mount_point(),flags);
}

booster::shared_ptr<application_specific_pool> 
applications_pool::get_application_specific_pool(char const *host,char const *script_name,char const *path_info,std::string &match)
{
	booster::unique_lock<booster::recursive_mutex> lock(d->lock);

	for(std::list<_data::attachment>::iterator it = d->apps.begin();it!=d->apps.end();++it) {
		std::pair<bool,std::string> m = it->mp.match(host,script_name,path_info);
		if(!m.first)
			continue;
		match = m.second;
		return it->pool;
	}
	booster::shared_ptr<application_specific_pool> result;
	for(std::list<_data::attachment>::iterator itr = d->legacy_async_apps.begin();itr!=d->legacy_async_apps.end();) {
		std::list<_data::attachment>::iterator app_it = itr;
		++itr;
		if(app_it->pool->flags() == -1) {
			d->legacy_async_apps.erase(app_it);
		}
		else if (!result) {
			std::pair<bool,std::string> m = app_it->mp.match(host,script_name,path_info);
			if(!m.first)
				continue;
			match = m.second;
			result = app_it->pool;
		}
	}
	return result;
}

booster::intrusive_ptr<application> applications_pool::get(	char const *,
								char const *,
								char const *,
								std::string &)
{
	throw cppcms_error("THIS IS INTERNAL MEMBER FUNCTION METHOD MUST NOT BE USED");
}

void applications_pool::put(application *)
{
	BOOSTER_WARNING("cppcms") << "CALL OF INTERNAL METHOD";
}


} //cppcms
