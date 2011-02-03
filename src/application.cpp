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

#include <set>
#include <vector>
#include <sstream>

#include <booster/locale/message.h>

#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms {

struct application::_data {
	_data(cppcms::service *s,application *app):
		service(s),
		pool_id(-1),
		url_map(app)
	{
	}
	cppcms::service *service;
	booster::shared_ptr<http::context> conn;
	int pool_id;
	url_dispatcher url;
	url_mapper url_map;
	std::vector<application *> managed_children;
};

application::application(cppcms::service &srv) :
	d(new _data(&srv,this)),
	refs_(0)
{
	parent_=root_=this;
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
	return d->url;
}

url_mapper &application::mapper()
{
	return d->url_map;
}

booster::shared_ptr<http::context> application::get_context()
{
	return root()->d->conn;
}

http::context &application::context()
{
	if(!root()->d->conn)
		throw cppcms_error("Access to unassigned context");
	return *root()->d->conn;
}

booster::shared_ptr<http::context> application::release_context()
{
	booster::shared_ptr<http::context> ptr=root()->d->conn;
	assign_context(booster::shared_ptr<http::context>());
	return ptr;
}


bool application::is_asynchronous()
{
	return pool_id() < 0;
}

void application::assign_context(booster::shared_ptr<http::context> conn)
{
	root()->d->conn=conn;
}

void application::pool_id(int id)
{
	d->pool_id=id;
}

int application::pool_id()
{
	return d->pool_id;
}

application *application::parent()
{
	return parent_;
}

application *application::root()
{
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

namespace {
	class rnd_guard {
	public:
		rnd_guard(base_content &cnt,application *app) : cnt_(&cnt)
		{
			cnt_->rendering_application(*app);
		}
		~rnd_guard()
		{
			cnt_->reset_rendering_application();
		}
	private:
		base_content *cnt_;
	};
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
	rnd_guard g(content,this);
	service().views_pool().render(context().skin(),template_name,response().out(),content);
}

void application::render(std::string skin,std::string template_name,base_content &content)
{
	rnd_guard g(content,this);
	service().views_pool().render(skin,template_name,response().out(),content);
}

void application::render(std::string template_name,std::ostream &out,base_content &content)
{
	rnd_guard g(content,this);
	service().views_pool().render(context().skin(),template_name,out,content);
}

void application::render(std::string skin,std::string template_name,std::ostream &out,base_content &content)
{
	rnd_guard g(content,this);
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
	return booster::locale::translate(ctx,message).str<char>(context().locale());
}
std::string application::translate(char const *message)
{
	return booster::locale::translate(message).str<char>(context().locale());
}
std::string application::translate(char const *ctx,char const *single,char const *plural,int n)
{
	return booster::locale::translate(ctx,single,plural,n).str<char>(context().locale());
}
std::string application::translate(char const *single,char const *plural,int n)
{
	return booster::locale::translate(single,plural,n).str<char>(context().locale());
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
		// it is called in destructors... So be very careful
		try {
			app = app->root();
			long refs=--(app->refs_);
			if(refs > 0)
				return;
			
			cppcms::service &service=app->service();

			try {
				app->recycle();
			}
			catch(...) {
				if(app->pool_id() < 0) {
					service.applications_pool().put(app);
				}
				else
					delete app;
				throw;
			}

			service.applications_pool().put(app);
			// return the application to pool... or delete it if "pooled"
		}
		catch(...) 
		{
			// FIXME LOG IT?
		}
	}
} // booster
