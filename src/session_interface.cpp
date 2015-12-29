///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/session_interface.h>
#include <cppcms/session_pool.h>
#include <cppcms/session_api.h>
#include <cppcms/util.h>
#include <cppcms/http_context.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/http_cookie.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/service.h>
#include <cppcms/json.h>
#include <cppcms/urandom.h>
#include <cppcms/base64.h>
#include <booster/log.h>

#include <iostream>

#include "cached_settings.h"
#include <set>
#include <sstream>
#include "string.h"

namespace cppcms {

	struct session_interface::_data {
		session_pool *pool;
		session_interface_cookie_adapter *adapter;
		_data() : pool(0), adapter(0) {}
	};

	struct session_interface::entry {
		std::string value;
		bool exposed;
		entry() : exposed(false) {}
		explicit entry(std::string const &s) : value(s), exposed(false) {}
		entry(std::string const &v,bool exp) : value(v) , exposed(exp) {}
		bool operator==(entry const &other) const 
		{
			return value==other.value && exposed==other.exposed;
		}
		bool operator!=(entry const &other) const 
		{
			return !(*this==other);
		}
	};

void session_interface::init()
{
	csrf_validation_ = cached_settings().security.csrf.enable;
	csrf_do_validation_ = cached_settings().security.csrf.automatic;
	timeout_val_def_=cached_settings().session.timeout;
	std::string s_how=cached_settings().session.expire;
	if(s_how=="fixed") {
		how_def_=fixed;
	}
	else if(s_how=="renew") {
		how_def_=renew;
	}
	else if(s_how=="browser") {
		how_def_=browser;
	}
	else {
		throw cppcms_error("Unsupported `session.expire' type `"+s_how+"'");
	}
}

session_interface::session_interface(session_pool &pool,session_interface_cookie_adapter &adapter) :
	context_(0),
	loaded_(0),
	reset_(0),
	csrf_checked_(0),
	csrf_do_validation_(0),
	csrf_validation_(0),
	d(new session_interface::_data())
{
	d->pool = &pool;
	d->adapter = &adapter;
	init();
	storage_=d->pool->get();
}

session_interface::session_interface(http::context &context) :
	context_(&context),
	loaded_(0),
	reset_(0),
	csrf_checked_(0),
	csrf_do_validation_(0),
	csrf_validation_(0),
	d(new session_interface::_data())
{
	init();
	storage_=context_->service().session_pool().get();
}

session_interface::~session_interface()
{
}

void session_interface::request_origin_validation_is_required(bool v)
{
	csrf_do_validation_ = v;
}

bool session_interface::validate_csrf_token(std::string const &token)
{
	std::string session_token = get("_csrf","");
	return session_token.empty() || session_token == token;
}

void session_interface::validate_request_origin()
{
	if(!context_)
		throw cppcms_error("request origin validation isn't possible without http::context");
	if(csrf_checked_)
		return;
	csrf_checked_ = 1;

	if(!csrf_validation_)
		return;
	if(!csrf_do_validation_)
		return;
	if(context_->request().request_method()!="POST")
		return;
	
	std::string token;
	
	typedef http::request::form_type::const_iterator iterator_type;
	std::pair<iterator_type,iterator_type> pair=context_->request().post().equal_range("_csrf");

	if(pair.first != pair.second && std::distance(pair.first,pair.second)==1)
		token = pair.first->second;
	else
		token = context_->request().getenv("HTTP_X_CSRFTOKEN");
	
	if(!validate_csrf_token(token)) {
		BOOSTER_WARNING("cppcms")	<<"CSRF validation failed"
						<<" IP="<< context_->request().remote_addr() 
						<<" SCRIPT_NAME=" << context_->request().script_name() 
						<<" PATH_INFO="<<context_->request().path_info();  
		throw request_forgery_error();
	}
}

bool session_interface::load()
{
	if(loaded_)
		return false; // FIXME
	loaded_ = 1;
	if(!storage_.get())
		return false;
	data_.clear();
	data_copy_.clear();
	timeout_val_=timeout_val_def_;
	how_=how_def_;
	std::string ar;
	saved_=0;
	on_server_=0;
	if(!storage_->load(*this,ar,timeout_in_)) {
		return false;
	}
	load_data(data_,ar);
	data_copy_=data_;
	if(is_set("_t"))
		timeout_val_=get<int>("_t");
	if(is_set("_h"))
		how_=get<int>("_h");
	if(is_set("_s"))
		on_server_=get<int>("_s");
	return true;
}
bool session_interface::set_cookie_adapter_and_reload(session_interface_cookie_adapter &adapter)
{
	d->adapter = &adapter;
	loaded_ = 0;
	return load();
}

int session_interface::cookie_age()
{
	if(how_==browser)
		return 0;
	if(how_==renew || ( how_==fixed && new_session_ ))
		return timeout_val_;
	return timeout_in_ - time(NULL);
}

time_t session_interface::session_age()
{
	if(how_==browser || how_==renew || (how_==fixed && new_session_))
		return timeout_val_ + time(NULL);
	return timeout_in_;
}

namespace {
struct packed {
        uint32_t key_size  : 10;
        uint32_t exposed   :  1;
        uint32_t data_size : 21;
	packed() {}
	packed(unsigned ks,bool exp, unsigned ds)
	{
		if(ks >=1024) 
			throw cppcms_error("session::save key too long");
		if(ds >= 1024 * 1024 * 2)
			throw cppcms_error("session::save value too long");
		key_size=ks;
		exposed = exp ? 1 : 0;
		data_size=ds;
	}
	packed(char const *start,char const *end)
	{
		if(start + 4 <= end ) {
			memcpy(this,start,4);
		}
		else
			throw cppcms_error("session::format violation -> pack");
	}
};

}


void session_interface::save_data(data_type const &data,std::string &s)
{
	s.clear();
	data_type::const_iterator p;
	for(p=data.begin();p!=data.end();++p) {
		packed header(p->first.size(),p->second.exposed,p->second.value.size());
		char *ptr=(char *)&header;
		s.append(ptr,ptr+sizeof(header));
		s.append(p->first.begin(),p->first.end());
		s.append(p->second.value.begin(),p->second.value.end());
	}
}

void session_interface::load_data(data_type &data,std::string const &s)
{
	data.clear();
	char const *begin=s.data(),*end=begin+s.size();
	while(begin < end) {
		packed p(begin,end);
		begin +=sizeof(p);
		if(end - begin >= int(p.key_size + p.data_size)) {
			std::string key(begin,begin+p.key_size);
			begin+=p.key_size;
			std::string val(begin,begin+p.data_size);
			begin+=p.data_size;
			entry &ent=data[key];
			ent.exposed = p.exposed;
			ent.value.swap(val);
		}
		else {
			throw cppcms_error("sessions::format violation data");
		}

	}

}



void session_interface::update_exposed(bool force)
{
	
	std::set<std::string> removed;
	for(data_type::iterator p=data_.begin();p!=data_.end();++p) {
		data_type::iterator p2=data_copy_.find(p->first);
		if(p->second.exposed && (force || p2==data_copy_.end() || !p2->second.exposed || p->second.value!=p2->second.value)){
			set_session_cookie(cookie_age(),p->second.value,p->first);
		}
		else if(!p->second.exposed && ((p2!=data_copy_.end() && p2->second.exposed) || force)) {
			removed.insert(p->first);
		}
	}
	for(data_type::iterator p=data_copy_.begin();p!=data_copy_.end();++p) {
		if(p->second.exposed && data_.find(p->first)==data_.end()) {
			removed.insert(p->first);
		}
	}

	if(cached_settings().session.cookies.remove_unknown_cookies) {
		std::string prefix = cached_settings().session.cookies.prefix + "_";
		if(!d->adapter) {
			typedef http::request::cookies_type cookies_type;
			cookies_type const &input_cookies = context_->request().cookies();

			for(cookies_type::const_iterator cp=input_cookies.begin();cp!=input_cookies.end();++cp) {
				if(cp->first.compare(0,prefix.size(),prefix)!=0)	
					continue;
				std::string key = cp->first.substr(prefix.size());
				if(removed.find(key)!=removed.end())
					continue;
				data_type::iterator ptr;
				if((ptr = data_.find(key))==data_.end() || !ptr->second.exposed) {
					removed.insert(key);
				}
			}
		}
		else {
			std::set<std::string> cookies = d->adapter->get_cookie_names();
			for(std::set<std::string>::const_iterator cp=cookies.begin();cp!=cookies.end();++cp) {
				if(cp->compare(0,prefix.size(),prefix)!=0)	
					continue;
				std::string key = cp->substr(prefix.size());
				if(removed.find(key)!=removed.end())
					continue;
				data_type::iterator ptr;
				if((ptr = data_.find(key))==data_.end() || !ptr->second.exposed) {
					removed.insert(key);
				}
			}
		}
	}

	for(std::set<std::string>::const_iterator p=removed.begin();p!=removed.end();++p)
		set_session_cookie(-1,"",*p);

}


std::string session_interface::generate_csrf_token()
{
	unsigned char binary[6];
	unsigned char text[16];
	urandom_device dev; 
	dev.generate(binary,sizeof(binary));
	unsigned char *text_begin = text;
	unsigned char *text_end = b64url::encode(binary,binary+sizeof(binary),text_begin);
	return std::string(text_begin,text_end);
}

void session_interface::save()
{
	if(storage_.get()==NULL || !loaded_ || saved_)
		return;
	check();
	new_session_  = (data_copy_.empty() && !data_.empty()) || reset_;
	if(data_.empty()) {
		if(get_session_cookie()!="")
			storage_->clear(*this);
		update_exposed(true);
		return;
	}


	if(new_session_ && cached_settings().security.csrf.enable)
	{
		set("_csrf",generate_csrf_token());
		if(cached_settings().security.csrf.exposed)
			expose("_csrf");
	}


	time_t now = time(NULL);

	bool force_update=false;
	if(data_==data_copy_ && !new_session_) {
		if(how_==fixed) {
			return;
		}
		if(how_==renew || how_==browser) {
			int64_t delta=now + timeout_val_ - timeout_in_;
			if(delta < timeout_val_ * 0.1) {// Less then 10% -- no renew need
				return;
			}
		}
		force_update=true;
	}

	std::string ar;
	save_data(data_,ar);
	
	temp_cookie_.clear();
	storage_->save(*this,ar,session_age(),new_session_,on_server_);
	set_session_cookie(cookie_age(),temp_cookie_);
	temp_cookie_.clear();

	update_exposed(force_update);	
	saved_=true;
}

void session_interface::check()
{
	if(storage_.get()==NULL)
		throw cppcms_error("Session storage backend is not loaded\n");
}

std::string &session_interface::operator[](std::string const &key)
{
	check();
	return data_[key].value;
}

void session_interface::erase(std::string const &key)
{
	check();
	data_.erase(key);
}

bool session_interface::is_set(std::string const &key)
{
	check();
	return data_.find(key)!=data_.end();
}

void session_interface::clear()
{
	check();
	data_.clear();
}

std::set<std::string> session_interface::key_set()
{
	check();
	std::set<std::string> r;
	for(data_type::const_iterator p=data_.begin();p!=data_.end();++p) {
		if(p->first.c_str()[0]=='_')
			continue;
		r.insert(p->first);
	}
	return r;
}

std::string session_interface::get(std::string const &key,std::string const &def)
{
	check();
	data_type::const_iterator p=data_.find(key);
	if(p==data_.end()) 
		return def;
	return p->second.value;
}

std::string session_interface::get(std::string const &key)
{
	check();
	data_type::const_iterator p=data_.find(key);
	if(p==data_.end()) 
		throw cppcms_error("Undefined session key "+key);
	return p->second.value;
}

void session_interface::set(std::string const &key,std::string const &v)
{
	check();
	data_[key].value=v;
}

void session_interface::clear_session_cookie()
{
	check();
	if(get_session_cookie()!="")
		set_session_cookie(-1,"");
}

bool session_interface::is_blocking()
{
	return storage_ && storage_->is_blocking();
}

void session_interface::set_session_cookie(int64_t age,std::string const &data,std::string const &key)
{
	if(data.empty())
		age=-1;
	std::string cookie_name=cached_settings().session.cookies.prefix;
	if(!key.empty()) {
		cookie_name+="_";
		cookie_name+=key;
	}
	std::string const &domain = cached_settings().session.cookies.domain;
	std::string const &path   = cached_settings().session.cookies.path;
	int time_shift = cached_settings().session.cookies.time_shift;
	bool use_age = cached_settings().session.cookies.use_age;
	bool use_exp = cached_settings().session.cookies.use_exp;

	bool secure = cached_settings().session.cookies.secure;

	http::cookie the_cookie(cookie_name,util::urlencode(data),path,domain);

	if(age < 0) {
		if(use_age)
			the_cookie.max_age(0);
		if(use_exp)
			the_cookie.expires(1);
	}
	else if(age == 0) {
		the_cookie.browser_age();
	}
	else {
		if(use_age)
			the_cookie.max_age(age);
		if(use_exp) {
			the_cookie.expires(age + time(0) + time_shift);
		}
	}


	the_cookie.secure(secure);
	
	if(d->adapter)
		d->adapter->set_cookie(the_cookie);
	else
		context_->response().set_cookie(the_cookie);
}

void session_interface::set_session_cookie(std::string const &data)
{
	check();
	temp_cookie_=data;
}

std::string session_interface::session_cookie_name()
{
	return cached_settings().session.cookies.prefix;
}

std::string session_interface::get_session_cookie()
{
	check();
	std::string const &name=cached_settings().session.cookies.prefix;
	if(d->adapter) {
		return d->adapter->get_session_cookie(name);
	}
	else {
		http::request::cookies_type const &cookies = context_->request().cookies();
		http::request::cookies_type::const_iterator p=cookies.find(name);
		if(p==cookies.end())
			return std::string();
		return p->second.value();
	}
}
	
bool session_interface::is_exposed(std::string const &key)
{
	data_type::iterator p=data_.find(key);
	if(p!=data_.end()) 
		return p->second.exposed;
	return false;
}

void session_interface::expose(std::string const &key,bool exp)
{
	data_[key].exposed=exp;
}

void session_interface::hide(std::string const &key)
{
	check();
	expose(key,false);
}

void session_interface::age(int t)
{
	check();
	timeout_val_=t;
	set("_t",t);
}

int session_interface::age()
{
	check();
	return timeout_val_;
}

void session_interface::default_age()
{
	check();
	erase("_t");
	timeout_val_=timeout_val_def_;
}

int session_interface::expiration()
{
	check();
	return how_;
}

void session_interface::expiration(int h)
{
	check();
	how_=h;
	set("_h",h);
}
void session_interface::default_expiration()
{
	check();
	erase("_h");
	how_=how_def_;
}

void session_interface::on_server(bool srv)
{
	check();
	on_server_=srv;
	set("_s",int(srv));
}

bool session_interface::on_server()
{
	check();
	return on_server_;
}

void session_interface::reset_session()
{
	reset_ = 1;
}

std::string session_interface::get_csrf_token()
{
	return get("_csrf","");
}

std::string session_interface::get_csrf_token_cookie_name()
{
	return cached_settings().session.cookies.prefix + "__csrf"; // one for suffix and one for _csrf
}

impl::cached_settings const &session_interface::cached_settings()
{
	if(context_)
		return context_->service().cached_settings();
	else
		return d->pool->cached_settings();
}

session_interface_cookie_adapter::~session_interface_cookie_adapter()
{
}


} // cppcms

