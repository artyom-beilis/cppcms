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
#include <cppcms/session_pool.h>
#include <cppcms/service.h>
#include <cppcms/thread_pool.h>
#include <cppcms/session_cookies.h>
#include <cppcms/session_sid.h>
#include <cppcms/session_dual.h>
#include "hmac_encryptor.h"
#include <cppcms/json.h>
#include <cppcms/cppcms_error.h>
#include <booster/shared_ptr.h>
#include <booster/enable_shared_from_this.h>
#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

#ifdef CPPCMS_HAVE_GCRYPT
#include "aes_encryptor.h"
#endif

#ifdef CPPCMS_WIN_NATIVE
#include "session_win32_file_storage.h"
#else
#include "session_posix_file_storage.h"
#endif
#include "session_memory_storage.h"

#include <booster/aio/deadline_timer.h>
#include <booster/callback.h>
#include <booster/posix_time.h>


namespace cppcms {
struct session_pool::_data 
{
};

using namespace cppcms::sessions;

template<typename Encryptor>
struct session_pool::enc_factory : public encryptor_factory {
	enc_factory(std::string const &key) : key_(key) {}
	virtual std::auto_ptr<cppcms::sessions::encryptor> get() 
	{
		std::auto_ptr<cppcms::sessions::encryptor> tmp(new Encryptor(key_));
		return tmp;
	}
private:
	std::string key_;
};

struct session_pool::sid_factory : public session_api_factory
{
	sid_factory(session_pool *pool) : pool_(pool) {}
	bool requires_gc() { 
		if(pool_->storage_.get())
			return pool_->storage_->requires_gc();
		else
			return false;
	}
	void gc() 
	{
		if(pool_->storage_.get())
			pool_->storage_->gc_job();
	}
	booster::shared_ptr<session_api> get() 
	{
		booster::shared_ptr<session_api> p;
		if(pool_->storage_.get())
			p.reset(new session_sid(pool_->storage_->get()));
		return p;
	}
private:
	session_pool *pool_;
};

struct session_pool::cookies_factory : public session_api_factory
{
	cookies_factory(session_pool *pool) : pool_(pool) {}
	bool requires_gc() {
		return false; 
	}
	void gc() {}
	booster::shared_ptr<session_api> get() 
	{
		booster::shared_ptr<session_api> p;
		if(pool_->encryptor_.get())
			p.reset(new session_cookies(pool_->encryptor_->get()));
		return p;
	}
private:
	session_pool *pool_;
};

struct session_pool::dual_factory : public session_api_factory
{
	dual_factory(unsigned limit,session_pool *pool) : limit_(limit), pool_(pool) {}
	bool requires_gc() { 
		if(pool_->storage_.get())
			return pool_->storage_->requires_gc();
		else
			return false;
	}
	void gc() 
	{
		if(pool_->storage_.get())
			pool_->storage_->gc_job();
	}
	booster::shared_ptr<session_api> get() 
	{
		booster::shared_ptr<session_api> p;
		if(pool_->storage_.get() && pool_->encryptor_.get())
			p.reset(new session_dual(pool_->encryptor_->get(),pool_->storage_->get(),limit_));
		return p;
	}
private:
	unsigned limit_;
	session_pool *pool_;
};


class session_pool::gc_job : public booster::enable_shared_from_this<gc_job> {
public:
	gc_job(service *ser,int freq,session_pool *pool) :
		timer_(new booster::aio::deadline_timer(ser->get_io_service())),
		service_(ser),
		freq_(freq),
		pool_(pool)
	{
	}
	void async_run() const
	{
		service_->thread_pool().post(boost::bind(&gc_job::gc,shared_from_this()));
	}
private:
	void gc() const 
	{
		pool_->backend_->gc();
		timer_->expires_from_now(booster::ptime(freq_));
		timer_->async_wait(boost::bind(&gc_job::async_run,shared_from_this()));
	}
	booster::shared_ptr<booster::aio::deadline_timer> timer_;
	service *service_;
	int freq_;
	session_pool *pool_;
};


void session_pool::init()
{
	service &srv=*service_;
	if(backend_.get())
		return;

	std::string location=srv.settings().get("session.location","none");

	if(location == "client" || (location=="both" && !encryptor_.get())) {
		std::string enc=srv.settings().get<std::string>("session.client.encryptor");
		std::auto_ptr<cppcms::sessions::encryptor_factory> factory;
		if(enc=="hmac") {
			std::string key = srv.settings().get<std::string>("session.client.key");
			factory.reset(new enc_factory<cppcms::sessions::impl::hmac_cipher>(key));
		}
		#ifdef CPPCMS_HAVE_GCRYPT
		else if(enc=="aes") {
			std::string key = srv.settings().get<std::string>("session.client.key");
			factory.reset(new enc_factory<cppcms::sessions::impl::aes_cipher>(key));
		}
		#endif
		else
			throw cppcms_error("Unknown encryptor: "+enc);
		encryptor(factory);
	}
	if(location == "server" || (location == "both" && !storage_.get())) {
		std::string stor=srv.settings().get<std::string>("session.server.storage");
		std::auto_ptr<sessions::session_storage_factory> factory;
		if(stor == "files") {
			std::string dir = srv.settings().get("session.server.dir","");
			#ifdef CPPCMS_WIN_NATIVE
			factory.reset(new session_file_storage_factory(dir));
			#else
			bool sharing = srv.settings().get("session.server.shared",true);
			int threads = srv.threads_no();
			int procs = srv.procs_no();
			if(procs == 0) procs=1;
			factory.reset(new session_file_storage_factory(dir,threads*procs,procs,sharing));
			#endif
		}
		else if(stor == "memory") {
			if(srv.procs_no() > 1)
				throw cppcms_error("Can't use memory storage with more then 1 worker process");
			factory.reset(new session_memory_storage_factory());
		}
		else 
			throw cppcms_error("Unknown server side storage:"+stor);
		storage(factory);
	}
	if(location == "server") {
		std::auto_ptr<session_api_factory> f(new sid_factory(this));
		backend(f);
	}
	else if(location == "client") {
		std::auto_ptr<session_api_factory> f(new cookies_factory(this));
		backend(f);
	}
	else if(location == "both") {
		unsigned limit=srv.settings().get("session.client_size_limit",2048);
		std::auto_ptr<session_api_factory> f(new dual_factory(limit,this));
		backend(f);
	}
	else if(location == "none")
		;
	else 
		throw cppcms_error("Unknown location");

	service_->after_fork(boost::bind(&session_pool::after_fork,this));
}

session_pool::session_pool(service &srv) :
	service_(&srv)
{
}

void session_pool::after_fork()
{
	if(backend_.get() && backend_->requires_gc()) {
		if(service_->process_id()!=1)
			return;
		int frequency = service_->settings().get("session.gc",0);
		if(frequency > 0) {
			booster::shared_ptr<gc_job> job(new gc_job(service_,frequency,this));
			job->async_run();
		}
	}
}

session_pool::~session_pool()
{
}

booster::shared_ptr<session_api> session_pool::get()
{
	booster::shared_ptr<session_api> p;
	if(backend_.get())
		p=backend_->get();
	return p;
}

void session_pool::backend(std::auto_ptr<session_api_factory> b)
{
	backend_=b;
}
void session_pool::encryptor(std::auto_ptr<sessions::encryptor_factory> e)
{
	encryptor_=e;
}
void session_pool::storage(std::auto_ptr<sessions::session_storage_factory> s)
{
	storage_=s;
}

} /// cppcms
