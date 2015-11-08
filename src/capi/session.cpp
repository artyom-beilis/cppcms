///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/capi/session.h>
#include <cppcms/session_pool.h>
#include <cppcms/session_interface.h>
#include <cppcms/http_cookie.h>
#include <cppcms/json.h>
#include <set>
#include <list>
#include <string>
#include <stdexcept>
#include <booster/hold_ptr.h>
#include <booster/nowide/fstream.h>
#include <stdio.h>

namespace {
	template<typename C>
	void handle(C *ptr)
	{
		if(!ptr)
			return;
		try {
			try {
				throw;
			}
			catch(std::exception const &e) {
				ptr->strerror = e.what();
				ptr->error=true;
			}
			catch(...) {
				ptr->strerror = "Unknown exception";
				ptr->error=true;
			}

		}
		catch(...) { ptr->error = true; } 
	}
	void check_str(char const *str) 
	{
		if(!str)
			throw std::runtime_error("String is null");
	}
} // namespace


extern "C" {


struct cppcms_capi_session_pool {
	cppcms_capi_session_pool() : error(false) {}
	std::string strerror;
	bool error;
	booster::hold_ptr<cppcms::session_pool> p;
};

#define TRY try
#define CATCH(x,ok,error) catch(...) { handle(x); return error; } return ok

struct cppcms_capi_session {
	
	void check() { 
		if(!p.get())
			throw std::runtime_error("Session is not initialized");
	}
	void check_loaded()
	{
		check();
		if(!loaded)
			throw std::runtime_error("Session is not loaded");
	}
	void check_loaded_unsaved()
	{
		check();
		if(!loaded)
			throw std::runtime_error("Session is not loaded");
		if(saved)
			throw std::runtime_error("Session is already saved - no changes allowed");
	}
	void check_saved()
	{
		if(!saved)
			throw std::runtime_error("Session is not saved");
	}
	bool loaded;
	bool saved;

	std::string strerror;
	bool error;
	booster::hold_ptr<cppcms::session_interface> p;
	std::set<std::string> key_set;
	std::set<std::string>::const_iterator key_set_ptr;
	std::string returned_value;

	struct cookie_adapter : public cppcms::session_interface_cookie_adapter {
		std::list<cppcms::http::cookie> cookies;
		std::list<cppcms::http::cookie>::const_iterator cookies_ptr;
		std::string value;
		virtual void set_cookie(cppcms::http::cookie const &updated_cookie) {
			cookies.push_back(updated_cookie);
		}
		virtual std::string get_session_cookie(std::string const &) {
			return value;
		}
	} adapter;


	cppcms_capi_session() : error(false) {
		key_set_ptr = key_set.begin();
		loaded = false;
		saved = false;
	}
};

struct cppcms_capi_cookie {
	std::string name;
	std::string value;
	std::string path;
	std::string domain;
	bool secure;
	bool has_expires;
	bool has_max_age;
	time_t expires;
	unsigned max_age;
	std::string header;
	std::string header_content;

	cppcms_capi_cookie(cppcms::http::cookie const &c) :
		name(c.name()),
		value(c.value()),
		path(c.path()),
		domain(c.domain()),
		secure(c.secure()),
		has_expires(c.expires_defined()),
		has_max_age(c.max_age_defined()),
		expires(c.expires()),
		max_age(c.max_age())
	{
		std::ostringstream ss;
		ss << c;
		header=ss.str();
		size_t pos = header.find(':');
		if(pos!=std::string::npos)
			header_content=header.substr(pos+1);
	}
};


cppcms_capi_session_pool *cppcms_capi_session_pool_new()
{
	try {
		return new cppcms_capi_session_pool();
	}catch(...) { return 0; }
}
void cppcms_capi_session_pool_delete(cppcms_capi_session_pool *pool)
{
	if(pool)
		delete pool;
}
char const *cppcms_capi_session_pool_strerror(cppcms_capi_session_pool *pool)
{
	return pool->strerror.c_str();
}

int cppcms_capi_session_pool_got_error(cppcms_capi_session_pool *pool)
{
	return pool->error;
}
void cppcms_capi_session_pool_clear_error(cppcms_capi_session_pool *pool)
{
	pool->error = false;
	pool->strerror = "";
}




int cppcms_capi_session_pool_init(cppcms_capi_session_pool *pool,char const *config_file)
{
	TRY {
		if(!pool)
			return -1;
		check_str(config_file);
		cppcms::json::value v;
		booster::nowide::ifstream f(config_file);	
		if(!f)
			throw std::runtime_error(std::string("Failed to open file ") + config_file);
		int line = 0;
		if(!v.load(f,true,&line)) {
			std::ostringstream ss;
			ss << "Failed to parse " << config_file << " syntax error in line " << line;
			throw std::runtime_error(ss.str());
		}
		pool->p.reset(new cppcms::session_pool(v));
		pool->p->init();
	} CATCH(pool,0,-1);
}

int cppcms_capi_session_pool_init_from_json(cppcms_capi_session_pool *pool,char const *json)
{
	try {
		if(!pool)
			return -1;
		check_str(json);
		cppcms::json::value v;
		std::istringstream f(json);
		int line = 0;
		if(!v.load(f,true,&line)) {
			std::ostringstream ss;
			ss << "Failed to parse json syntax error in line " << line;
			throw std::runtime_error(ss.str());
		}
		pool->p.reset(new cppcms::session_pool(v));
		pool->p->init();
	} CATCH(pool,0,-1);
}


cppcms_capi_session *cppcms_capi_session_new()
{
	try {
		return new cppcms_capi_session();
	}
	catch(...) { return 0; }
}
void cppcms_capi_session_delete(cppcms_capi_session *session)
{
	if(session) delete session;
}
char const *cppcms_capi_session_strerror(cppcms_capi_session *session)
{
	return session->strerror.c_str();
}

int cppcms_capi_session_got_error(cppcms_capi_session *s)
{
	return s->error;
}
void cppcms_capi_session_clear_error(cppcms_capi_session *s)
{
	s->error = false;
	s->strerror = "";
}


int cppcms_capi_session_init(cppcms_capi_session *session,cppcms_capi_session_pool *pool)
{
	TRY {
		if(!pool->p.get()) {
			throw std::runtime_error("Session pool is not initialized");
		}
		session->p.reset(new cppcms::session_interface(*pool->p,session->adapter));
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_clear(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->clear();
	}CATCH(session,0,-1);
}

int cppcms_capi_session_is_set(cppcms_capi_session *session,char const *key)
{
	TRY {
		session->check_loaded();
		return session->p->is_set(key);
	}CATCH(session,0,-1);
}
int cppcms_capi_session_erase(cppcms_capi_session *session,char const *key)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->erase(key);
	}CATCH(session,0,-1);
}
int cppcms_capi_session_get_exposed(cppcms_capi_session *session,char const *key)
{
	TRY {
		session->check_loaded();
		return session->p->is_exposed(key);
	}CATCH(session,0,-1);
}
int cppcms_capi_session_set_exposed(cppcms_capi_session *session,char const *key,int is_exposed)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->expose(key,is_exposed);
	}CATCH(session,0,-1);
}

char const *cppcms_capi_session_get_first_key(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded();
		session->key_set = session->p->key_set();
		session->key_set_ptr = session->key_set.begin();
		if(session->key_set_ptr != session->key_set.end()) {
			char const *r=session->key_set_ptr->c_str();
			++session->key_set_ptr;
			return r;
		}
	}CATCH(session,0,0);
}
char const *cppcms_capi_session_get_next_key(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded();
		if(session->key_set_ptr != session->key_set.end()) {
			char const *r=session->key_set_ptr->c_str();
			++session->key_set_ptr;
			return r;
		}
	}
	CATCH(session,0,0);
}

char const *cppcms_capi_session_get_csrf_token(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded();
		session->returned_value = session->p->get_csrf_token();
		return session->returned_value.c_str();
	}
	CATCH(session,0,0);

}


int cppcms_capi_session_set(cppcms_capi_session *session,char const *key,char const *value,int value_len)
{
	TRY {
		session->check_loaded_unsaved();
		std::string v;
		if(value_len  == -1)
			v=value;
		else
			v.assign(value,size_t(value_len));
		session->p->set(key,v);
	}CATCH(session,0,-1);
}

char const *cppcms_capi_session_get(cppcms_capi_session *session,char const *key)
{
	TRY {
		session->check_loaded();
		if(!session->p->is_set(key))
			return 0;
		session->returned_value = session->p->get(key);
		return session->returned_value.c_str();
	}CATCH(session,0,0);

}
int cppcms_capi_session_get_len(cppcms_capi_session *session,char const *key)
{
	TRY {
		session->check_loaded();
		if(!session->p->is_set(key))
			return 0;
		return session->p->get(key).size();
	}CATCH(session,0,-1);

}

int cppcms_capi_session_reset_session(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->reset_session();
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_set_default_age(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->default_age();
	}
	CATCH(session,0,-1);
}
int cppcms_capi_session_set_age(cppcms_capi_session *session,int t)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->age(t);
	}
	CATCH(session,0,-1);
}
int cppcms_capi_session_get_age(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded();
		return session->p->age();
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_set_default_expiration(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->default_expiration();
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_set_expiration(cppcms_capi_session *session,int t)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->expiration(t);
	}
	CATCH(session,0,-1);
}
int cppcms_capi_session_get_expiration(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded();
		return session->p->expiration();
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_set_on_server(cppcms_capi_session *session,int is_on_server)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->on_server(is_on_server);
	}
	CATCH(session,0,-1);
}
int cppcms_capi_session_get_on_server(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded();
		return session->p->on_server();
	}
	CATCH(session,0,-1);
}

char const *cppcms_capi_session_get_session_cookie_name(cppcms_capi_session *session)
{
	TRY {
		session->check();
		session->returned_value = session->p->session_cookie_name();
		return session->returned_value.c_str();
	}
	CATCH(session,0,0);
}
int cppcms_capi_session_load(cppcms_capi_session *session,char const *session_cookie_value)
{
	TRY {
		session->check();
		if(session->loaded) {
			throw std::runtime_error("Session is already loaded");
		}
		session->adapter.value = session_cookie_value;
		session->p->load();
		session->loaded = true;
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_save(cppcms_capi_session *session)
{
	TRY {
		session->check_loaded_unsaved();
		session->p->save();
		session->saved = true;
		session->adapter.cookies_ptr=session->adapter.cookies.begin();
	}
	CATCH(session,0,-1);
}

cppcms_capi_cookie *cppcms_capi_session_cookie_first(cppcms_capi_session *session)
{
	TRY {
		session->check_saved();
		session->adapter.cookies_ptr=session->adapter.cookies.begin();
		if(session->adapter.cookies_ptr == session->adapter.cookies.end())
			return 0;
		cppcms_capi_cookie *r=new cppcms_capi_cookie(*session->adapter.cookies_ptr++);
		return r;
	}
	CATCH(session,0,0);
}

cppcms_capi_cookie *cppcms_capi_session_cookie_next(cppcms_capi_session *session)
{
	TRY {
		session->check_saved();
		if(session->adapter.cookies_ptr == session->adapter.cookies.end())
			return 0;
		cppcms_capi_cookie *r=new cppcms_capi_cookie(*session->adapter.cookies_ptr++);
		return r;
	}
	CATCH(session,0,0);
}

void cppcms_capi_cookie_delete(cppcms_capi_cookie *cookie) { delete cookie; }

char const *cppcms_capi_cookie_header(cppcms_capi_cookie const *cookie) { return cookie->header.c_str(); }
char const *cppcms_capi_cookie_header_content(cppcms_capi_cookie const *cookie) { return cookie->header_content.c_str(); }

char const *cppcms_capi_cookie_name(cppcms_capi_cookie const *cookie) { return cookie->name.c_str(); }
char const *cppcms_capi_cookie_value(cppcms_capi_cookie const *cookie) { return cookie->value.c_str(); }
char const *cppcms_capi_cookie_path(cppcms_capi_cookie const *cookie) { return cookie->path.c_str(); }
char const *cppcms_capi_cookie_domain(cppcms_capi_cookie const *cookie) { return cookie->domain.c_str(); }

int cppcms_capi_cookie_max_age_defined(cppcms_capi_cookie const *cookie) { return cookie->has_max_age; }
unsigned cppcms_capi_cookie_max_age(cppcms_capi_cookie const *cookie) { return cookie->max_age; }

int cppcms_capi_cookie_expires_defined(cppcms_capi_cookie const *cookie) { return cookie->has_expires; }
long long cppcms_capi_cookie_expires(cppcms_capi_cookie const *cookie) { return cookie->expires; }

int cppcms_capi_cookie_expires_is_secure(cppcms_capi_cookie const *cookie) { return cookie->secure; }

} // extern "C"
