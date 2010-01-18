#define CPPCMS_SOURCE

#include "application.h"
#include "http_context.h"
#include "service.h"
#include "cppcms_error.h"
#include "url_dispatcher.h"
#include "intrusive_ptr.h"
#include "applications_pool.h"
#include "http_response.h"
#include "views_pool.h"

#include <set>
#include <vector>

#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms {

struct application::data {
	data(cppcms::service *s):
		service(s),
		conn(0),
		pool_id(-1)
	{
	}
	cppcms::service *service;
	intrusive_ptr<http::context> conn;
	int pool_id;
	url_dispatcher url;
	std::vector<application *> managed_children;
};

application::application(cppcms::service &srv) :
	d(new data(&srv)),
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

long application::refs()
{
	return refs_;
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

intrusive_ptr<http::context> application::get_context()
{
	return root()->d->conn;
}

http::context &application::context()
{
	if(!root()->d->conn)
		throw cppcms_error("Access to unassigned context");
	return *root()->d->conn;
}

intrusive_ptr<http::context> application::release_context()
{
	intrusive_ptr<http::context> ptr=root()->d->conn;
	assign_context(0);
	return ptr;
}


bool application::is_asynchronous()
{
	return pool_id() < 0;
}

void application::assign_context(intrusive_ptr<http::context> conn)
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
void application::add(application &app,std::string regex,int part)
{
	add(app);
	url_dispatcher().mount(regex,app,part);
}
void application::attach(application *app)
{
	d->managed_children.push_back(app);
	add(*app);
}

void application::main(std::string url)
{
	if(!dispatcher().dispatch(url)) {
		response().make_error_response(http::response::not_found);
	}
}

void application::attach(application *app,std::string regex,int part)
{
	d->managed_children.push_back(app);
	add(*app,regex,part);
}

void application::render(std::string template_name,base_content &content)
{
	service().views_pool().render(context().skin(),template_name,response().out(),content);
}

void application::render(std::string skin,std::string template_name,base_content &content)
{
	service().views_pool().render(skin,template_name,response().out(),content);
}

void application::render(std::string template_name,std::ostream &out,base_content &content)
{
	service().views_pool().render(context().skin(),template_name,out,content);
}

void application::render(std::string skin,std::string template_name,std::ostream &out,base_content &content)
{
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
	if(root()->d->conn) {
		response().finalize();
		context().async_complete_response();
	}
	assign_context(0);
}

void intrusive_ptr_add_ref(application *app)
{
	++(app->root()->refs_);
}

// REMEMBER THIS IS CALLED FROM DESTRUCTOR!!!
void intrusive_ptr_release(application *app)
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


} // cppcms
