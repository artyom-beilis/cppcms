#define CPPCMS_SOURCE

#include "application.h"
#include "http_context.h"
#include "service.h"
#include "cppcms_error.h"
#include "url_dispatcher.h"
#include "locale_environment.h"
#include "locale_gettext.h"
#include "intrusive_ptr.h"
#include "applications_pool.h"
#include "http_response.h"

#include <set>
#include <vector>

#include <boost/bind.hpp>

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
	typedef std::set<intrusive_ptr<http::context> > all_conn_type;
	all_conn_type all_conn;
	int pool_id;
	url_dispatcher url;
	std::vector<application *> managed_children;
};

application::application(cppcms::service &srv,application *p) :
	d(new data(&srv)),
	refs_(0)
{
	if(p==0)
		parent_=root_=this;
	else
		parent(p);	
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


http::context &application::context()
{
	if(!root()->d->conn)
		throw cppcms_error("Access to unassigned context");
	return *root()->d->conn;
}

http::context *application::last_assigned_context()
{
	return &*root()->d->conn;
}


cppcms::locale::environment &application::locale()
{
	return context().locale();
}

char const *application::gt(char const *s)
{
	return locale().gt(s);
}

char const *application::ngt(char const *s,char const *p,int n)
{
	return locale().ngt(s,p,n);
}
bool application::is_asynchronous()
{
	return pool_id() < 0;
}

void application::assign_context(intrusive_ptr<http::context> conn)
{
	if(parent()!=this)
		throw cppcms_error("Can assign context to only root of applications tree");
	if(conn == 0)
		d->conn = 0;
	else {
		d->conn=conn;
		d->all_conn.insert(conn);
	}
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
void application::assign(application *app)
{
	d->managed_children.push_back(app);
	add(*app);
}


void application::assign(application *app,std::string regex,int part)
{
	d->managed_children.push_back(app);
	add(*app,regex,part);
}

void application::release_context(http::context *conn)
{
	intrusive_ptr<http::context> context=conn;
	context->response().out() << std::flush;
	context->response().finalize();
	context->service().post(boost::bind(&http::context::on_response_complete,context));
	root()->d->all_conn.erase(context);
}

void application::release_all_contexts()
{
	root()->d->conn=0;
	for(data::all_conn_type::iterator p=root()->d->all_conn.begin();p!=root()->d->all_conn.end();++p) {
		intrusive_ptr<http::context> context=*p;
		context->response().out() << std::flush;
		context->response().finalize();
		context->service().post(boost::bind(&http::context::on_response_complete,context));
	}
	root()->d->all_conn.clear();
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
			app->release_all_contexts();
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
