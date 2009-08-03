#define CPPCMS_SOURCE

#include "application.h"
#include "http_context.h"
#include "service.h"
#include "cppcms_error.h"
#include "url_dispatcher.h"


namespace cppcms {

struct application::data {
	data(cppcms::service *s):
		service(s),
		conn(0),
		pool_id(-1)
	{
	}
	cppcms::service *service;
	http::context *conn;
	int pool_id;
	url_dispatcher url;
};

application::application(cppcms::service &srv) :
	d(new data(&srv))
{

}

application::~application()
{
}

cppcms::service &application::service()
{
	return *d->service;
}

cppcms_config const &application::settings() 
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
	if(!d->conn)
		throw cppcms_error("Trying to access uninitialized context");
	return *d->conn;
}

void application::assign_context(http::context *conn)
{
	d->conn=conn;
}

void application::pool_id(int id)
{
	d->pool_id=id;
}

int application::pool_id()
{
	return d->pool_id;
}



} // cppcms
