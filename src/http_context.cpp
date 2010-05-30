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
#include "cgi_api.h"
#include <cppcms/service.h>
#include <cppcms/http_context.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/thread_pool.h>
#include <cppcms/views_pool.h>
#include <cppcms/cache_interface.h>
#include <cppcms/session_interface.h>
#include <cppcms/cppcms_error.h>
#include <booster/log.h>

#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif


namespace cppcms {
namespace http {

	struct context::_data {
		std::locale locale;
		std::string skin;
		http::request request;
		std::auto_ptr<http::response> response;
		std::auto_ptr<cache_interface> cache;
		std::auto_ptr<session_interface> session;
		_data(context &cntx) :
			locale(cntx.connection().service().locale()),
			request(cntx.connection())
		{
		}
	};

context::context(booster::shared_ptr<impl::cgi::connection> conn) :
	conn_(conn)
{
	d.reset(new _data(*this));
	d->response.reset(new http::response(*this));
	skin(service().views_pool().default_skin());
	d->cache.reset(new cache_interface(*this));
	d->session.reset(new session_interface(*this));
}

std::string context::skin()
{
	return d->skin;
}

cache_interface &context::cache()
{
	return *d->cache;
}

void context::skin(std::string const &skin)
{
	d->skin=skin;
}


void context::run()
{
	conn_->async_prepare_request(this,boost::bind(&context::on_request_ready,self(),_1));
}

void context::on_request_ready(bool error)
{
	if(error) return;
	
	std::string host = conn_->getenv("HTTP_HOST");
	std::string path_info = conn_->getenv("PATH_INFO");
	std::string script_name = conn_->getenv("SCRIPT_NAME");
	std::string matched;

	booster::intrusive_ptr<application> app = service().applications_pool().get(host,script_name,path_info,matched);

	if(!app) {
		response().io_mode(http::response::asynchronous);
		response().make_error_response(http::response::not_found);
		async_complete_response();
		return;
	}

	app->assign_context(self());
	
	if(app->is_asynchronous()) {
		response().io_mode(http::response::asynchronous);
		app->service().post(boost::bind(&context::dispatch,app,matched,false));
	}
	else {
		app->service().thread_pool().post(boost::bind(&context::dispatch,app,matched,true));
	}
}
// static 
void context::dispatch(booster::intrusive_ptr<application> app,std::string url,bool syncronous)
{
	try {
		if(syncronous)
			app->context().session().load();
		app->main(url);
	}
	catch(std::exception const &e){
		if(app->get_context() && !app->response().some_output_was_written()) {
			app->response().make_error_response(http::response::internal_server_error,e.what());
		}
		else {
			BOOSTER_ERROR("cppcms") << "Catched excepion ["<<e.what()<<"]";
		}
	}
}

namespace {
	void wrapper(context::handler const &h,bool r)
	{
		h(r ? context::operation_aborted : context::operation_completed);
	}
}


void context::async_flush_output(context::handler const &h)
{
	if(response().io_mode() != http::response::asynchronous) {
		throw cppcms_error("Can't use asynchronouse operations when I/O mode is synchronous");
	}
	conn_->async_write_response(
		response(),
		false,
		boost::bind(wrapper,h,_1));
}

void context::async_complete_response()
{
	if(response().io_mode() == http::response::asynchronous || response().io_mode() == http::response::asynchronous_raw) {
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
		booster::shared_ptr<context> cont(new context(conn_));
		cont->run();
	}
	conn_.reset();
}

booster::shared_ptr<context> context::self()
{
	return shared_from_this();
}

context::~context()
{
}

void context::async_on_peer_reset(booster::function<void()> const &h)
{
	// For some wired can't go without bind on SunCC
	conn_->aync_wait_for_close_by_peer(boost::bind(h));
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
	return *d->response;
}

json::value const &context::settings()
{
	return conn_->service().settings();
}

std::locale context::locale()
{
	return d->locale;
}
void context::locale(std::locale const &new_locale)
{
	d->locale=new_locale;
	if(response().some_output_was_written())
		response().out().imbue(d->locale);
}
void context::locale(std::string const &name)
{
	locale(service().locale(name));
}
session_interface &context::session()
{
	return *d->session;
}

} // http
} // cppcms


