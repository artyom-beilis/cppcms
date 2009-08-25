#define CPPCMS_SOURCE

#include "cgi_api.h"
#include "service.h"
#include "http_context.h"
#include "http_request.h"
#include "http_response.h"
#include "locale_environment.h"
#include "application.h"
#include "applications_pool.h"
#include "thread_pool.h"
#include "url_dispatcher.h"

#include <boost/bind.hpp>


namespace cppcms {
namespace http {

	struct context::data {
		http::request request;
		http::response response;
		cppcms::locale::environment locale;
		data(context &cntx) :
			request(cntx.connection()),
			response(cntx),
			locale(cntx.connection().service())
		{
		}
	};

context::context(intrusive_ptr<impl::cgi::connection> conn) :
	conn_(conn)
{
	d.reset(new data(*this));	
}


void context::run()
{
	conn_->async_prepare_request(d->request,boost::bind(&context::on_request_ready,self(),_1));
}

void context::on_request_ready(bool error)
{
	if(error) return;
	
	std::string path_info = conn_->getenv("PATH_INFO");
	std::string script_name = conn_->getenv("SCRIPT_NAME");
	std::string matched;

	intrusive_ptr<application> app = service().applications_pool().get(script_name,path_info,matched);

	url_dispatcher::dispatch_type how;
	bool make_404 = !app || ((how=app->dispatcher().dispatchable(matched))==url_dispatcher::none);

	if(make_404) {
		app=0;
		response().io_mode(http::response::asynchronous);
		response().make_error_response(http::response::not_found);
		on_response_complete();
		return;
	}

	app->assign_context(self());
	bool sync = !app->is_asynchronous() && (how != url_dispatcher::asynchronous);
	if(sync) {
		app->service().thread_pool().post(boost::bind(&context::dispatch,app,true));
	}
	else {
		response().io_mode(http::response::asynchronous);
		app->service().post(boost::bind(&context::dispatch,app,false));
	}
}

/* static */
void context::dispatch(intrusive_ptr<application> app,bool syncronous)
{
	try {
		app->dispatcher().dispatch();
	}
	catch(std::exception const &e){
		if(!app->response().some_output_was_written()) {
			app->response().make_error_response(http::response::internal_server_error,e.what());
		}
		else {
			// TODO log it
			std::cerr<<"Catched excepion ["<<e.what()<<"]"<<std::endl;
		}
	}
}

void context::on_response_complete()
{
	if(response().io_mode() == http::response::asynchronous) {
		conn_->async_write_response(
			response(),
			true,
			boost::bind(&context::try_restart,self(),_1));
		return;
	}
	conn_->async_complete_response(boost::bind(&context::try_restart,self(),_1));
}

void context::try_restart(bool e)
{
	if(e) return;

	if(conn_->is_reuseable()) {
		intrusive_ptr<context> cont=new context(conn_);
		cont->run();
	}
	conn_=0;
}

intrusive_ptr<context> context::self()
{
	intrusive_ptr<context> ptr(this);
	return ptr;
}

context::~context()
{
}


impl::cgi::connection &context::connection()
{
	return *conn_;
}

cppcms::service &context::service()
{
	return conn_->service();
}

http::request &context::request()
{
	return d->request;
}

http::response &context::response()
{
	return d->response;
}

json::value const &context::settings()
{
	return conn_->service().settings();
}

cppcms::locale::environment &context::locale()
{
	return d->locale;
}


} // http
} // cppcms
