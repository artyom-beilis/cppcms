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
#include <typeinfo>

extern "C" {

struct cppcms_capi_exception {
	int code;
	char const *msg;
	std::string s_msg;
	char c_msg[64];
	cppcms_capi_exception() : code(0), msg("ok")
	{
		memset(c_msg,0,sizeof(c_msg));
	}
};



struct cppcms_capi_session_pool {
	cppcms_capi_exception e;
	booster::hold_ptr<cppcms::session_pool> p;

};

struct cppcms_capi_session {
	cppcms_capi_exception e;
	
	void check() { 
		if(!p.get())
			throw std::logic_error("Session is not initialized");
	}
	void check_loaded()
	{
		check();
		if(!loaded)
			throw std::logic_error("Session is not loaded");
	}
	void check_loaded_unsaved()
	{
		check();
		if(!loaded)
			throw std::logic_error("Session is not loaded");
		if(saved)
			throw std::logic_error("Session is already saved - no changes allowed");
	}
	void check_saved()
	{
		if(!saved)
			throw std::logic_error("Session is not saved");
	}
	bool loaded;
	bool saved;

	booster::hold_ptr<cppcms::session_interface> p;
	std::set<std::string> key_set;
	std::set<std::string>::const_iterator key_set_ptr;
	std::string returned_value;

	struct cookie_adapter : public cppcms::session_interface_cookie_adapter {
		std::map<std::string,cppcms::http::cookie> cookies;
		std::map<std::string,cppcms::http::cookie>::const_iterator cookies_ptr;

		std::string value;
		std::set<std::string> keys;
		virtual void set_cookie(cppcms::http::cookie const &updated_cookie) {
			cookies[updated_cookie.name()]=updated_cookie;
		}
		virtual std::string get_session_cookie(std::string const &) {
			return value;
		}
		virtual std::set<std::string> get_cookie_names()
		{
			return keys;
		}
	} adapter;


	cppcms_capi_session() {
		key_set_ptr = key_set.begin();
		loaded = false;
		saved = false;
	}
};

struct cppcms_capi_cookie {
	cppcms_capi_exception e;
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





} // extern C


namespace {

	void seterror(cppcms_capi_exception &c,int code,std::exception const &e) 
	{
		if(c.code)
			return;
		c.code = code;
		try {
			c.s_msg = e.what();
			c.msg = c.s_msg.c_str();
		}
		catch(...) {
			strncpy(c.c_msg,e.what(),sizeof(c.c_msg)-1);
			c.msg = c.c_msg;
		}
	}
	void seterror(cppcms_capi_exception &c,int code,char const *msg) 
	{
		if(c.code)
			return;
		c.code = code;
		c.msg=msg;
	}

	template<typename C>
	void handle(C *ptr)
	{
		if(!ptr)
			return;
		try {
			try {
				throw;
			}
			catch(std::runtime_error const &e) {
				seterror(ptr->e,CPPCMS_CAPI_ERROR_RUNTIME,e);
			}
			catch(std::invalid_argument const &e) {
				seterror(ptr->e,CPPCMS_CAPI_ERROR_INVALID_ARGUMENT,e);
			}
			catch(std::logic_error const &e) {
				seterror(ptr->e,CPPCMS_CAPI_ERROR_LOGIC,e);
			}
			catch(std::bad_alloc const &e) {
				seterror(ptr->e,CPPCMS_CAPI_ERROR_ALLOC,"memory allocation error");
			}
			catch(std::exception const &e) {
				seterror(ptr->e,CPPCMS_CAPI_ERROR_GENERAL,e);
			}
			catch(...) {
				seterror(ptr->e,CPPCMS_CAPI_ERROR_GENERAL,"unknown error");
			}

		}
		catch(...) { 
			seterror(ptr->e,CPPCMS_CAPI_ERROR_GENERAL,"unknown error");
		} 
	}
	void check_str(char const *str) 
	{
		if(!str)
			throw std::invalid_argument("String is null");
	}
	unsigned char hex_to_digit(char c) 
	{
		if('0' <= c && c<='9')
			return c - '0';
		if('a' <= c && c<= 'f')
			return c - 'a' + 10;
		if('A' <= c && c<= 'F')
			return c - 'A' + 10;
		throw std::invalid_argument("Non hexadecimal digit detected");
	}
} // namespace


extern "C" {

#define TRY try
#define CATCH(x,ok,error) catch(...) { handle(x); return error; } return ok

int cppcms_capi_error(cppcms_capi_object obj)
{
	if(!obj)
		return 0;
	cppcms_capi_exception *e=static_cast<cppcms_capi_exception *>(obj);
	return e->code;
}
char const *cppcms_capi_error_message(cppcms_capi_object obj)
{
	if(!obj)
		return "ok";
	cppcms_capi_exception *e=static_cast<cppcms_capi_exception *>(obj);
	if(e->code == 0)
		return "ok";
	else
		return e->msg;
}
char const *cppcms_capi_error_clear(cppcms_capi_object obj)
{
	char const *r=cppcms_capi_error_message(obj);
	if(obj) {
		static_cast<cppcms_capi_exception *>(obj)->code = 0;
	}
	return r;
}




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

int cppcms_capi_session_init(cppcms_capi_session *session,cppcms_capi_session_pool *pool)
{
	TRY {
		if(!session)
			return -1;
		if(!pool)
			throw std::logic_error("pool is NULL");
		if(!pool->p.get()) {
			throw std::logic_error("Session pool is not initialized");
		}
		session->p.reset(new cppcms::session_interface(*pool->p,session->adapter));
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_clear(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded_unsaved();
		session->p->clear();
	}CATCH(session,0,-1);
}

int cppcms_capi_session_is_set(cppcms_capi_session *session,char const *key)
{
	TRY {
		if(!session)
			return -1;
		check_str(key);
		session->check_loaded();
		return session->p->is_set(key);
	}CATCH(session,0,-1);
}
int cppcms_capi_session_erase(cppcms_capi_session *session,char const *key)
{
	TRY {
		if(!session)
			return -1;
		check_str(key);
		session->check_loaded_unsaved();
		session->p->erase(key);
	}CATCH(session,0,-1);
}
int cppcms_capi_session_get_exposed(cppcms_capi_session *session,char const *key)
{
	TRY {
		if(!session)
			return -1;
		check_str(key);
		session->check_loaded();
		return session->p->is_exposed(key);
	}CATCH(session,0,-1);
}
int cppcms_capi_session_set_exposed(cppcms_capi_session *session,char const *key,int is_exposed)
{
	TRY {
		if(!session)
			return -1;
		check_str(key);
		session->check_loaded_unsaved();
		session->p->expose(key,is_exposed);
	}CATCH(session,0,-1);
}

char const *cppcms_capi_session_get_first_key(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return 0;
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
		if(!session)
			return 0;
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
		if(!session)
			return 0;
		session->check_loaded();
		session->returned_value = session->p->get_csrf_token();
		return session->returned_value.c_str();
	}
	CATCH(session,0,0);

}


int cppcms_capi_session_set(cppcms_capi_session *session,char const *key,char const *value)
{
	TRY {
		if(!session)
			return -1;
		check_str(key);
		check_str(value);
		session->check_loaded_unsaved();
		(*(session->p))[key]=value;
	}CATCH(session,0,-1);
}

char const *cppcms_capi_session_get(cppcms_capi_session *session,char const *key)
{
	TRY {
		if(!session)
			return 0;
		check_str(key);
		session->check_loaded();
		if(!session->p->is_set(key))
			return 0;
		session->returned_value = session->p->get(key);
		return session->returned_value.c_str();
	}CATCH(session,0,0);

}

int cppcms_capi_session_get_binary_len(cppcms_capi_session *session,char const *key)
{
	TRY {
		if(!session)
			return -1;
		check_str(key);
		session->check_loaded();
		if(!session->p->is_set(key))
			return 0;
		return (*(session->p))[key].size();
	}CATCH(session,0,-1);
}

int cppcms_capi_session_set_binary(cppcms_capi_session *session,char const *key,void const *value,int length)
{
	TRY {
		if(!session)
			return -1;
		check_str(key);
		if(!value)
			throw std::invalid_argument("value is null");
		if(length < 0)
			throw std::invalid_argument("length is negative");
		session->check_loaded_unsaved();
		(*(session->p))[key].assign(static_cast<char const *>(value),size_t(length));
	}CATCH(session,0,-1);
}
int cppcms_capi_session_get_binary(cppcms_capi_session *session,char const *key,void *buf,int buffer_size)
{

	TRY {
		if(!session)
			return -1;
		check_str(key);
		if(buffer_size < 0)
			throw std::invalid_argument("buffer size is negative");
		if(!buf)
			throw std::invalid_argument("buffer is null");
		session->check_loaded();
		if(!session->p->is_set(key))
			return 0;
		std::string &value = (*(session->p))[key];
		int copy_size = value.size();
		if(copy_size > buffer_size)
			throw std::invalid_argument("Output buffer is too small");
		memcpy(buf,value.c_str(),copy_size);
		return copy_size;
	}CATCH(session,0,-1);

}


int cppcms_capi_session_set_binary_as_hex(cppcms_capi_session *session,char const *key,char const *value)
{
	TRY {
		if(!session)
			return -1;
		check_str(key);
		check_str(value);
		int len = strlen(value);
		if(len % 2 != 0)
			throw std::invalid_argument("value lengths is odd");
		std::string tmp;
		tmp.reserve(len / 2);
		for(int i=0;i<len;i+=2) {
			unsigned char h=hex_to_digit(value[i]);
			unsigned char l=hex_to_digit(value[i+1]);
			unsigned char b = h * 16u  + l;
			tmp+=char(b);
		}
		
		session->check_loaded_unsaved();
		(*(session->p))[key].swap(tmp);

	}CATCH(session,0,-1);
}
char const *cppcms_capi_session_get_binary_as_hex(cppcms_capi_session *session,char const *key)
{
	TRY {
		if(!session)
			return 0;
		check_str(key);
		session->check_loaded();

		if(!session->p->is_set(key))
			return 0;
		std::string const &value = (*(session->p))[key];
		std::string tmp;
		int len = value.size();
		tmp.reserve(len*2);
		for(int i=0;i<len;i++) {
			static char const *digits="0123456789abcdef";
			unsigned char c=value[i];
			tmp+=digits[(c >> 4u & 0xFu)];
			tmp+=digits[(c & 0xFu)];
		}
		session->returned_value.swap(tmp);
		return session->returned_value.c_str();
	}CATCH(session,0,0);
}


int cppcms_capi_session_reset_session(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded_unsaved();
		session->p->reset_session();
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_set_default_age(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded_unsaved();
		session->p->default_age();
	}
	CATCH(session,0,-1);
}
int cppcms_capi_session_set_age(cppcms_capi_session *session,int t)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded_unsaved();
		session->p->age(t);
	}
	CATCH(session,0,-1);
}
int cppcms_capi_session_get_age(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded();
		return session->p->age();
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_set_default_expiration(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded_unsaved();
		session->p->default_expiration();
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_set_expiration(cppcms_capi_session *session,int t)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded_unsaved();
		session->p->expiration(t);
	}
	CATCH(session,0,-1);
}
int cppcms_capi_session_get_expiration(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded();
		return session->p->expiration();
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_set_on_server(cppcms_capi_session *session,int is_on_server)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded_unsaved();
		session->p->on_server(is_on_server);
	}
	CATCH(session,0,-1);
}
int cppcms_capi_session_get_on_server(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return -1;
		session->check_loaded();
		return session->p->on_server();
	}
	CATCH(session,0,-1);
}

char const *cppcms_capi_session_get_session_cookie_name(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return 0;
		session->check();
		session->returned_value = session->p->session_cookie_name();
		return session->returned_value.c_str();
	}
	CATCH(session,0,0);
}

int cppcms_capi_session_set_session_cookie(cppcms_capi_session *session,char const *session_cookie_value)
{
	TRY {
		if(!session)
			return -1;
		check_str(session_cookie_value);
		session->check();
		session->adapter.value = session_cookie_value;
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_add_cookie_name(cppcms_capi_session *session,char const *name)
{
	TRY {
		if(!session)
			return -1;
		check_str(name);
		session->check();
		session->adapter.keys.insert(name);
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_load(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return -1;
		session->check();
		if(session->loaded) {
			throw std::logic_error("Session is already loaded");
		}
		session->p->load();
		session->loaded = true;
	}
	CATCH(session,0,-1);
}

int cppcms_capi_session_save(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return -1;
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
		if(!session)
			return 0;
		session->check_saved();
		session->adapter.cookies_ptr=session->adapter.cookies.begin();
		if(session->adapter.cookies_ptr == session->adapter.cookies.end())
			return 0;
		cppcms_capi_cookie *r=new cppcms_capi_cookie(session->adapter.cookies_ptr->second);
		++session->adapter.cookies_ptr;
		return r;
	}
	CATCH(session,0,0);
}

cppcms_capi_cookie *cppcms_capi_session_cookie_next(cppcms_capi_session *session)
{
	TRY {
		if(!session)
			return 0;
		session->check_saved();
		if(session->adapter.cookies_ptr == session->adapter.cookies.end())
			return 0;
		cppcms_capi_cookie *r=new cppcms_capi_cookie(session->adapter.cookies_ptr->second);
		++session->adapter.cookies_ptr;
		return r;
	}
	CATCH(session,0,0);
}

void cppcms_capi_cookie_delete(cppcms_capi_cookie *cookie) { if(cookie) delete cookie; }

char const *cppcms_capi_cookie_header(cppcms_capi_cookie const *cookie) { return cookie ? cookie->header.c_str() : 0; }
char const *cppcms_capi_cookie_header_content(cppcms_capi_cookie const *cookie) { return cookie ? cookie->header_content.c_str() : 0; }

char const *cppcms_capi_cookie_name(cppcms_capi_cookie const *cookie) { return cookie ? cookie->name.c_str(): 0 ; }
char const *cppcms_capi_cookie_value(cppcms_capi_cookie const *cookie) { return cookie ? cookie->value.c_str(): 0; }
char const *cppcms_capi_cookie_path(cppcms_capi_cookie const *cookie) { return cookie ? cookie->path.c_str(): 0; }
char const *cppcms_capi_cookie_domain(cppcms_capi_cookie const *cookie) { return cookie ? cookie->domain.c_str() : 0; }

int cppcms_capi_cookie_max_age_defined(cppcms_capi_cookie const *cookie) { return cookie ? cookie->has_max_age : -1; }
unsigned cppcms_capi_cookie_max_age(cppcms_capi_cookie const *cookie) { return cookie ? cookie->max_age: 0; }

int cppcms_capi_cookie_expires_defined(cppcms_capi_cookie const *cookie) { return cookie ? cookie->has_expires: -1; }
long long cppcms_capi_cookie_expires(cppcms_capi_cookie const *cookie) { return cookie ? cookie->expires: -1; }

int cppcms_capi_cookie_is_secure(cppcms_capi_cookie const *cookie) { return cookie ? cookie->secure: -1; }

} // extern "C"
