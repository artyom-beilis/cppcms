///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE

#include <cppcms/application.h>
#include <cppcms/http_context.h>
#include <cppcms/service.h>
#include <cppcms/filters.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/applications_pool.h>
#include <cppcms/http_response.h>
#include <cppcms/views_pool.h>
#include <booster/hold_ptr.h>

#include <set>
#include <vector>
#include <sstream>

#include <booster/locale/message.h>

#include <cppcms/config.h>

namespace cppcms {

struct application::_data {
	_data(cppcms::service *s):
		service(s),
		temp_conn(0)
	{
	}
	cppcms::service *service;
	booster::shared_ptr<http::context> conn;
	http::context *temp_conn;
	booster::hold_ptr<url_dispatcher> url_disp;
	booster::hold_ptr<url_mapper> url_map;
	std::vector<application *> managed_children;
	booster::weak_ptr<application_specific_pool> my_pool;
};

application::application(cppcms::service &srv) :
	d(new _data(&srv)),
	refs_(0)
{
	parent_=root_=this;
	d->url_disp.reset(new url_dispatcher(this));
	d->url_map.reset(new url_mapper(this));
}

application::~application()
{
	for(unsigned i=0;i<d->managed_children.size();i++) {
		delete d->managed_children[i];
		d->managed_children[i]=0;
	}
}

cppcms::service &application::service()
{
	return *d->service;
}

json::value const &application::settings() 
{
	return service().settings();
}

http::request &application::request()
{
	return context().request();
}
http::response &application::response()
{
	return context().response();
}

url_dispatcher &application::dispatcher()
{
	return *d->url_disp;
}

url_mapper &application::mapper()
{
	return *d->url_map;
}

booster::shared_ptr<http::context> application::get_context()
{
	return root()->d->conn;
}

void application::add_context(http::context &conn)
{
	if(root()->d->conn)
		throw cppcms_error("Context already assigned");
	root()->d->temp_conn = &conn;
}

void application::remove_context()
{
	root()->d->temp_conn = 0;
}

http::context &application::context()
{
	if(!root()->d->conn) {
		if(root()->d->temp_conn)
			return *root()->d->temp_conn;
		throw cppcms_error("Access to unassigned context");
	}
	return *root()->d->conn;
}

bool application::has_context()
{
	return root()->d->conn || root()->d->temp_conn;
}

bool application::owns_context()
{
	return root()->d->conn;
}

booster::shared_ptr<http::context> application::release_context()
{
	booster::shared_ptr<http::context> ptr=root()->d->conn;
	assign_context(booster::shared_ptr<http::context>());
	return ptr;
}


bool application::is_asynchronous()
{
	booster::shared_ptr<application_specific_pool> p=d->my_pool.lock();
	if(p && (p->flags() & app::op_mode_mask) != 0)
		return true;
	return false; 
}

void application::assign_context(booster::shared_ptr<http::context> conn)
{
	root()->d->conn=conn;
	root()->d->temp_conn = 0;
}

void application::set_pool(booster::weak_ptr<application_specific_pool> p)
{
	d->my_pool = p;
}

booster::weak_ptr<application_specific_pool> application::get_pool()
{
	return d->my_pool;
}

application *application::parent()
{
	return parent_;
}

application *application::root()
{
	if(root_ == root_->root_)
		return root_;
	do {
		root_ = root_->root_;
	}
	while(root_->root_ != root_);
	return root_;
}

void application::parent(application *app)
{
	parent_=app;
	root_=app->root();
}


void application::add(application &app)
{
	if(app.parent()!=this)
		app.parent(this);
}
void application::add(application &app,std::string const &regex,int part)
{
	add(app);
	dispatcher().mount(regex,app,part);
}

void application::add(application &app,std::string const &name,std::string const &url)
{
	add(app);
	mapper().mount(name,url,app);
}

void application::add(application &app,std::string const &name,std::string const &url,std::string const &regex,int part)
{
	add(app);
	dispatcher().mount(regex,app,part);
	mapper().mount(name,url,app);
}


void application::attach(application *app)
{
	d->managed_children.push_back(app);
	add(*app);
}

void application::init()
{
}

void application::clear()
{
}


void application::main(std::string url)
{
	if(!dispatcher().dispatch(url)) {
		response().make_error_response(http::response::not_found);
	}
}

void application::attach(application *app,std::string const &regex,int part)
{
	attach(app);
	dispatcher().mount(regex,*app,part);
}
void application::attach(application *app,std::string const &name,std::string const &url)
{
	attach(app);
	mapper().mount(name,url,*app);
}
void application::attach(application *app,std::string const &name,std::string const &url,std::string const &regex,int part)
{
	attach(app);
	dispatcher().mount(regex,*app,part);
	mapper().mount(name,url,*app);
}

void application::render(std::string template_name,base_content &content)
{
	base_content::app_guard g(content,*this);
	service().views_pool().render(context().skin(),template_name,response().out(),content);
}

void application::render(std::string skin,std::string template_name,base_content &content)
{
	base_content::app_guard g(content,*this);
	service().views_pool().render(skin,template_name,response().out(),content);
}

void application::render(std::string template_name,std::ostream &out,base_content &content)
{
	base_content::app_guard g(content,*this);
	service().views_pool().render(context().skin(),template_name,out,content);
}

void application::render(std::string skin,std::string template_name,std::ostream &out,base_content &content)
{
	base_content::app_guard g(content,*this);
	service().views_pool().render(skin,template_name,out,content);
}

cache_interface &application::cache()
{
	return context().cache();
}

session_interface &application::session()
{
	return context().session();
}


void application::recycle()
{
	assign_context(booster::shared_ptr<http::context>());
}

std::string application::translate(char const *ctx,char const *message)
{
	return booster::locale::translate(ctx,message).str(context().locale());
}
std::string application::translate(char const *message)
{
	return booster::locale::translate(message).str(context().locale());
}
std::string application::translate(char const *ctx,char const *single,char const *plural,int n)
{
	return booster::locale::translate(ctx,single,plural,n).str(context().locale());
}
std::string application::translate(char const *single,char const *plural,int n)
{
	return booster::locale::translate(single,plural,n).str(context().locale());
}

std::string application::url(std::string const &key)
{
	std::ostringstream ss;
	ss.imbue(context().locale());
	mapper().map(ss,key);
	return ss.str();
}

std::string application::url(	std::string const &key,
				filters::streamable const &p1)
{
	std::ostringstream ss;
	ss.imbue(context().locale());
	mapper().map(ss,key,p1);
	return ss.str();
}

std::string application::url(	std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2)
{
	std::ostringstream ss;
	ss.imbue(context().locale());
	mapper().map(ss,key,p1,p2);
	return ss.str();
}

std::string application::url(	std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3)
{
	std::ostringstream ss;
	ss.imbue(context().locale());
	mapper().map(ss,key,p1,p2,p3);
	return ss.str();
}

std::string application::url(	std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4)
{
	std::ostringstream ss;
	ss.imbue(context().locale());
	mapper().map(ss,key,p1,p2,p3,p4);
	return ss.str();
}

std::string application::url(	std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5)
{
	std::ostringstream ss;
	ss.imbue(context().locale());
	mapper().map(ss,key,p1,p2,p3,p4,p5);
	return ss.str();
}

std::string application::url(	std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5,
				filters::streamable const &p6)
{
	std::ostringstream ss;
	ss.imbue(context().locale());
	mapper().map(ss,key,p1,p2,p3,p4,p5,p6);
	return ss.str();
}



} // cppcms

namespace booster {
	void intrusive_ptr_add_ref(cppcms::application *app)
	{
		++(app->root()->refs_);
	}

	// REMEMBER THIS IS CALLED FROM DESTRUCTOR!!!
	void intrusive_ptr_release(cppcms::application *app)
	{
		if(!app)
			return;
		try {
			app = app->root();
			long refs=--(app->refs_);
			if(refs > 0)
				return;
			app->recycle();
			booster::shared_ptr<cppcms::application_specific_pool> p = app->get_pool().lock();
			if(p) {
				cppcms::application *tmp = app;
				app = 0;
				p->put(tmp);
			}
			else
				delete app;
		}
		catch(...) 
		{
			if(app)
				delete app;
		}
	}
} // booster
