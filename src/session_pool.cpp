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
#include <booster/shared_object.h>
#include <booster/enable_shared_from_this.h>
#include <cppcms/config.h>
#include <cppcms_boost/bind.hpp>
namespace boost = cppcms_boost;

#include "aes_encryptor.h"

#ifdef CPPCMS_WIN_NATIVE
#include "session_win32_file_storage.h"
#else
#include "session_posix_file_storage.h"
#endif

#include "session_memory_storage.h"
#ifndef CPPCMS_NO_TCP_CACHE
#include "session_tcp_storage.h"
#endif

#include <booster/aio/deadline_timer.h>
#include <booster/callback.h>
#include <booster/posix_time.h>


namespace cppcms {
struct session_pool::_data 
{
	booster::shared_object module;
};

using namespace cppcms::sessions;

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
	gc_job(service *ser,double freq,session_pool *pool) :
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
		booster::ptime start = booster::ptime::now();
		booster::ptime restart = start + booster::ptime::from_number(freq_);
		pool_->backend_->gc();
		timer_->expires_at(restart);
		timer_->async_wait(boost::bind(&gc_job::async_run,shared_from_this()));
	}
	booster::shared_ptr<booster::aio::deadline_timer> timer_;
	service *service_;
	double freq_;
	session_pool *pool_;
};


void session_pool::init()
{
	service &srv=*service_;
	if(backend_.get())
		return;

	std::string location=srv.settings().get("session.location","none");

	if((location == "client" || location=="both") && !encryptor_.get()) {
		using namespace cppcms::sessions::impl;
		std::string enc=srv.settings().get("session.client.encryptor","");
		std::string mac=srv.settings().get("session.client.hmac","");
		std::string cbc=srv.settings().get("session.client.cbc","");
		
		if(enc.empty() &&  mac.empty() && cbc.empty()) {
			throw cppcms_error("Using clinet session storage without encryption method");
		}
		if(!enc.empty() && (!mac.empty() || !cbc.empty())) {
			throw cppcms_error("Can't specify both session.client.encryptor and session.client.hmac or session.client.cbc");
		}
		if(!cbc.empty() && mac.empty()) {
			throw cppcms_error("Can't use session encryption without MAC");
		}

		std::auto_ptr<cppcms::sessions::encryptor_factory> factory;
		if(!enc.empty()) {
			crypto::key k;

			std::string key_file = srv.settings().get("session.client.key_file","");

			if(key_file.empty())
				k=crypto::key(srv.settings().get<std::string>("session.client.key"));
			else
				k.read_from_file(key_file);

			if(enc=="hmac") {
				factory.reset(new hmac_factory("sha1",k));
			}
			else if(enc.compare(0,5,"hmac-") == 0) {
				std::string algo = enc.substr(5);
				factory.reset(new hmac_factory(algo,k));
			}
			else if(enc.compare(0,3,"aes") == 0) {
				factory.reset(new aes_factory(enc,k));
			}
			else
				throw cppcms_error("Unknown encryptor: "+enc);
		}
		else {
			crypto::key hmac_key;
			std::string hmac_key_file = srv.settings().get("session.client.hmac_key_file","");
			if(hmac_key_file.empty())
				hmac_key = crypto::key(srv.settings().get<std::string>("session.client.hmac_key"));
			else
				hmac_key.read_from_file(hmac_key_file);

			if(cbc.empty()) {
				factory.reset(new hmac_factory(mac,hmac_key));
			}
			else {
				crypto::key cbc_key;
				std::string cbc_key_file = srv.settings().get("session.client.cbc_key_file","");

				if(cbc_key_file.empty())
					cbc_key = crypto::key(srv.settings().get<std::string>("session.client.cbc_key"));
				else
					cbc_key.read_from_file(cbc_key_file);

				factory.reset(new aes_factory(cbc,cbc_key,mac,hmac_key));
			}
		}

		encryptor(factory);
	}
	if((location == "server" || location == "both") && !storage_.get()) {
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
		else if(stor == "external") {
			std::string so = srv.settings().get<std::string>("session.server.shared_object","");
			std::string module = srv.settings().get<std::string>("session.server.module","");
			std::string entry_point = srv.settings().get<std::string>("session.server.entry_point","sessions_generator");
			if(so.empty() && module.empty())
				throw cppcms_error(	"sessions_pool: session.storage=external "
							"and neither session.server.shared_object "
							"nor session.server.module is defined");
			if(!so.empty() && !module.empty())
				throw cppcms_error(	"sessions_pool: both session.server.shared_object "
							"and session.server.module are defined");

			if(so.empty()) {
				so = booster::shared_object::name(module);
			}
			std::string error;
			if(!d->module.open(so,error)) {
				throw cppcms_error("sessions_pool: failed to load shared object " + so + ": " + error);
			}
			cppcms_session_storage_generator_type f=0;
			d->module.symbol(f,entry_point);
			factory.reset(f(srv.settings().find("session.server.settings")));
		}
#ifndef CPPCMS_NO_TCP_CACHE
		else if(stor == "network") {
			typedef std::vector<std::string> ips_type;
			typedef std::vector<int> ports_type;
			ips_type ips = srv.settings().get<ips_type>("session.server.ips");
			ports_type ports = srv.settings().get<ports_type>("session.server.ports");
			if(ips.size() != ports.size())
				throw cppcms_error(	"sessions_pool: session.server.ips and "
							"session.server.ports are not of the same size");
			if(ips.empty()) {
				throw cppcms_error("sessions_pool:session.server.ips is empty");
			}
			factory.reset(new tcp_factory(ips,ports));
		}
#endif
		else 
			throw cppcms_error("sessions_pool: unknown server side storage:"+stor);
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
	d(new _data()),
	service_(&srv)
{
}

void session_pool::after_fork()
{
	if(backend_.get() && backend_->requires_gc()) {
		if(service_->process_id()!=1)
			return;
		double frequency = service_->settings().get("session.gc",0.0);
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
