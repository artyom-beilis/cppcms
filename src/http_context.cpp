///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
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
#include <booster/backtrace.h>
#include <booster/aio/io_service.h>
#include <cppcms/http_content_filter.h>
#include <stdio.h>

#include "cached_settings.h"

#include <cppcms/config.h>
#include "binder.h"


namespace cppcms {
namespace http {

	struct context::_data {
		std::locale locale;
		std::string skin;
		http::request request;
		std::auto_ptr<http::response> response;
		std::auto_ptr<cache_interface> cache;
		std::auto_ptr<session_interface> session;
		booster::shared_ptr<application_specific_pool> pool;
		booster::intrusive_ptr<application> app;
		std::string matched;
		booster::hold_ptr<context::holder> specific;
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
}

void context::set_holder(holder *p)
{
	d->specific.reset(p);
}
context::holder *context::get_holder()
{
	return d->specific.get();
}

std::string context::skin()
{
	return d->skin;
}

cache_interface &context::cache()
{
	if(!d->cache.get()) {
		d->cache.reset(new cache_interface(*this));
	}
	return *d->cache;
}

void context::skin(std::string const &skin)
{
	d->skin=skin;
}


namespace {
	struct ct_to_bool {
		void (context::*member)(bool r);
		booster::shared_ptr<context> ctx;
		void operator()(context::completion_type c)
		{
			((*ctx).*member)(c!=context::operation_completed);
		}
	};
}

void context::run()
{
	ct_to_bool cb = { &context::on_request_ready, self() };
	conn_->async_prepare_request(this,cb);
}

namespace {
	struct dispatcher {
		void (*func)(booster::shared_ptr<application_specific_pool> const &,booster::shared_ptr<context> const &,std::string const &);
		booster::shared_ptr<application_specific_pool> pool;
		booster::shared_ptr<context> ctx;
		std::string url;
		void operator()() {
			func(pool,ctx,url); 
		}
	};
}


void context::submit_to_pool(booster::shared_ptr<application_specific_pool> pool,std::string const &matched)
{
	submit_to_pool_internal(pool,matched,false);
}


namespace {
	struct dispatch_binder {
		void (*dispatch)(booster::intrusive_ptr<application> const &,std::string const &,bool);
		booster::shared_ptr<context> ctx;
		booster::intrusive_ptr<application> app;
		std::string matched;
		bool flag;

		void operator()()
		{
			app->assign_context(ctx);
			dispatch(app,matched,flag);
		}

	};

	class context_guard {
	public:
		context_guard(cppcms::application *app,cppcms::http::context &ctx) : app_(app)
		{
			if(app_)
				app_->add_context(ctx);
		}
		~context_guard() 
		{
			if(app_)
				app_->remove_context();
		}
	private:	
		cppcms::application *app_;
	};
}



void context::submit_to_asynchronous_application(booster::intrusive_ptr<application> app,std::string const &matched)
{
	dispatch_binder bd = { &context::dispatch, self(), app,matched,false };
	conn_->get_io_service().post(bd);
}


void context::submit_to_pool_internal(booster::shared_ptr<application_specific_pool> pool,std::string const &matched,bool now)
{
	if((pool->flags() & app::op_mode_mask)!=app::synchronous) {
		// asynchronous 
		booster::intrusive_ptr<application> app = pool->get(service());

		if(!app) {
			BOOSTER_ERROR("cppcms") << "Cound fetch asynchronous application from pool";
			response().io_mode(http::response::asynchronous);
			response().make_error_response(http::response::internal_server_error);
			async_complete_response();
			return;
		}
		if(now) {
			app->assign_context(self());
			response().io_mode(http::response::asynchronous);
			dispatch(app,matched,false);
		}
		else {
			submit_to_asynchronous_application(app,matched);
		}

		return;
	}
	else {
		dispatcher dt;
		dt.func = &context::dispatch;
		dt.pool = pool;
		dt.ctx = self();
		dt.url=matched;
		service().thread_pool().post(dt);
		return;
	}
}

int context::translate_exception()
{
	try {
		throw;
	}
	catch(abort_upload const &e) {
		return e.code();
	}
	catch(std::exception const &e) {
		make_error_message(e);
		return 500;
	}
	catch(...) {
		BOOSTER_ERROR("cppcms") << "Unknown exception";
		return 500;
	}
	return 0;
}

int context::on_headers_ready()
{
	char const *host = conn_->cgetenv("HTTP_HOST");
	char const *path_info = conn_->cgetenv("PATH_INFO");
	char const *script_name = conn_->cgetenv("SCRIPT_NAME");
	std::string matched;

	booster::intrusive_ptr<application> app;
	booster::shared_ptr<application_specific_pool> pool = 
		service().applications_pool().get_application_specific_pool(
			host,
			script_name,
			path_info,
			matched
			);
	if(!pool)
		return 404;
	
	request().prepare();

	int flags;
	
	if(request().content_length() != 0 && ((flags=pool->flags()) & app::op_mode_mask) != app::synchronous && (flags & app::content_filter)!=0) {
		app = pool->get(service());
		if(!app)
			return 500;
		try {
			context_guard g(app.get(),*this);
			app->main(matched);
		}
		catch(...) {
			return translate_exception();
		}
	}

	d->pool.swap(pool);
	d->matched.swap(matched);
	d->app.swap(app);
	
	return request().on_content_start();
}

int context::on_content_progress(size_t n)
{
	context_guard g(d->app.get(),*this);
	return request().on_content_progress(n);
}

void context::on_request_ready(bool error)
{
	booster::shared_ptr<application_specific_pool> pool;
	booster::intrusive_ptr<application> app;
	pool.swap(d->pool);
	app.swap(d->app);

	if(error) {
		if(app) {
			try {
				context_guard g(app.get(),*this);
				request().on_error();
			}
			catch(std::exception const &e) {
				BOOSTER_ERROR("cppcms") << "exception at request::on_error" << e.what() << booster::trace(e);
			}
			catch(...) {
				BOOSTER_ERROR("cppcms") << "Unknown exception at request::on_error";
			}
		}
		return;
	}

	if(app) {
		app->assign_context(self());
		dispatch(app,d->matched,false);
		return;
	}

	submit_to_pool_internal(pool,d->matched,true);
}

namespace {
	struct run_ctx {  
		booster::shared_ptr<context> ctx;
		void operator()() {
			ctx->run();
		}
	};
}

void context::complete_response()
{
	response().finalize();
	if(conn_->is_reuseable()) {
		booster::shared_ptr<context> cont(new context(conn_));
		run_ctx rn = { cont };
		service().post(rn);
	}
	conn_.reset();
}
// static 
void context::dispatch(booster::shared_ptr<application_specific_pool> const &pool,booster::shared_ptr<context> const &self,std::string const &url)
{
	booster::intrusive_ptr<application> app = pool->get(self->service());
	if(!app) {
		BOOSTER_ERROR("cppcms") << "Cound fetch synchronous application from a pool";
		self->response().make_error_response(http::response::internal_server_error);
		self->complete_response();
		return;
	}
	app->assign_context(self);
	dispatch(app,url,true);
}

void context::make_error_message(std::exception const &e)
{
	BOOSTER_ERROR("cppcms") << "Caught exception ["<<e.what()<<"]\n" << booster::trace(e)  ;
	if(!response().some_output_was_written()) {
		if(service().cached_settings().security.display_error_message) {
			std::ostringstream ss;
			ss << e.what() << '\n';
			ss << booster::trace(e);
			response().make_error_response(http::response::internal_server_error,ss.str());
		}
		else
			response().make_error_response(http::response::internal_server_error);
	}
}
// static 
void context::dispatch(booster::intrusive_ptr<application> const &app,std::string const &url,bool syncronous)
{
	try {
		if(syncronous) {
			app->response().io_mode(http::response::normal);
			if(!app->context().service().cached_settings().session.disable_automatic_load)
				app->context().session().load();
		}
		else {
			app->response().io_mode(http::response::asynchronous);
		}

		if(syncronous && !app->context().service().cached_settings().session.disable_automatic_load)
			app->context().session().load();
		app->main(url);
	}
	catch(request_forgery_error const &e) {
		if(app->get_context() && !app->response().some_output_was_written()) {
			app->response().make_error_response(http::response::forbidden);
		}
	}
	catch(...){
		if(app->get_context())
			app->context().translate_exception();
	}
	
	if(app->get_context()) {
		try {
			if(syncronous) {
				app->context().complete_response();
			}
			else  {
				app->context().async_complete_response();
			}
		}
		catch(...) {
			app->release_context();
			throw;
		}
		app->release_context();
	}
}

void context::async_flush_output(context::handler const &h)
{
	if(response().io_mode() != http::response::asynchronous && response().io_mode()!=http::response::asynchronous_raw) {
		throw cppcms_error("Can't use asynchronouse operations when I/O mode is synchronous");
	}
	conn_->async_write_response(
		response(),
		false,
		h);
}


void context::async_complete_response()
{
	response().finalize();
	if(response().io_mode() == http::response::asynchronous || response().io_mode() == http::response::asynchronous_raw) {
		ct_to_bool cb = { &context::try_restart, self() };
		conn_->async_write_response(
			response(),
			true,
			cb);
		return;
	}
	complete_response();
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

void context::async_on_peer_reset(booster::callback<void()> const &h)
{
	conn_->aync_wait_for_close_by_peer(h);
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
	if(!d->session.get())
		d->session.reset(new session_interface(*this));
	return *d->session;
}

} // http
} // cppcms


