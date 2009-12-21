#define CPPCMS_SOURCE
#include "session_interface.h"
#include "session_pool.h"
#include "session_api.h"
#include "util.h"
#include "http_context.h"
#include "http_request.h"
#include "http_response.h"
#include "http_cookie.h"
#include "cppcms_error.h"
#include "service.h"
#include "json.h"

#include <sstream>
#include "string.h"

using namespace std;

namespace cppcms {

	struct session_interface::data {};

	struct session_interface::entry {
		std::string value;
		bool exposed;
		entry(std::string v="",bool exp=false) : value(v) , exposed(exp) {}
		bool operator==(entry const &other) const 
		{
			return value==other.value && exposed==other.exposed;
		}
		bool operator!=(entry const &other) const 
		{
			return !(*this==other);
		}
	};

session_interface::session_interface(http::context &context) :
	context_(&context),
	loaded_(0)
{
	timeout_val_def_=context_->settings().get("session.timeout",24*3600);
	string s_how=context_->settings().get("session.expire","browser");
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

	storage_=context_->service().session_pool().get();
}

session_interface::~session_interface()
{
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
	string ar;
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
			string key(begin,begin+p.key_size);
			begin+=p.key_size;
			string val(begin,begin+p.data_size);
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
	for(data_type::iterator p=data_.begin();p!=data_.end();++p) {
		data_type::iterator p2=data_copy_.find(p->first);
		if(p->second.exposed && (force || p2==data_copy_.end() || !p2->second.exposed || p->second.value!=p2->second.value)){
			set_session_cookie(cookie_age(),p->second.value,p->first);
		}
		else if(!p->second.exposed && ((p2!=data_copy_.end() && p2->second.exposed) || force)) {
			set_session_cookie(-1,"",p->first);
		}
	}
	for(data_type::iterator p=data_copy_.begin();p!=data_copy_.end();++p) {
		if(p->second.exposed && data_.find(p->first)==data_.end()) {
			set_session_cookie(-1,"",p->first);
		}
	}
}

void session_interface::save()
{
	if(storage_.get()==NULL || !loaded_ || saved_)
		return;
	check();
	new_session_  = data_copy_.empty() && !data_.empty();
	if(data_.empty()) {
		if(get_session_cookie()!="")
			storage_->clear(*this);
		update_exposed(true);
		return;
	}

	time_t now = time(NULL);

	bool force_update=false;
	if(data_==data_copy_) {
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

	string ar;
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

string &session_interface::operator[](string const &key)
{
	check();
	return data_[key].value;
}

void session_interface::erase(string const &key)
{
	check();
	data_.erase(key);
}

bool session_interface::is_set(string const &key)
{
	check();
	return data_.find(key)!=data_.end();
}

void session_interface::clear()
{
	check();
	data_.clear();
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
	data_[key]=v;
}

void session_interface::clear_session_cookie()
{
	check();
	if(get_session_cookie()!="")
		set_session_cookie(-1,"");
}
void session_interface::set_session_cookie(int64_t age,string const &data,string const &key)
{
	if(data.empty())
		age=-1;
	std::string cookie_name=context_->settings().get("session.cookies_prefix","cppcms_session");
	if(!key.empty()) {
		cookie_name+="_";
		cookie_name+=key;
	}
	std::string domain = context_->settings().get("session.cookies_domain","");
	std::string path   = context_->settings().get("session.cookies_path","/");
	bool secure = context_->settings().get("session.cookies_secure",0);

	http::cookie the_cookie(cookie_name,util::urlencode(data),path,domain);

	if(age < 0) {
		the_cookie.max_age(0);
	}
	else if(age == 0)
		the_cookie.browser_age();
	else
		the_cookie.max_age(age);
	the_cookie.secure(secure);
	context_->response().set_cookie(the_cookie);
}

void session_interface::set_session_cookie(string const &data)
{
	check();
	temp_cookie_=data;
}

string session_interface::get_session_cookie()
{
	check();
	string name=context_->settings().get("session.cookies_prefix","cppcms_session");
	http::request::cookies_type const &cookies = context_->request().cookies();
	http::request::cookies_type::const_iterator p=cookies.find(name);
	if(p==cookies.end())
		return "";
	return p->second.value();
}
	
bool session_interface::is_exposed(std::string const &key)
{
	data_type::iterator p=data_.find(key);
	if(p!=data_.end()) 
		return p->second.exposed;
	return false;
}

void session_interface::expose(string const &key,bool exp)
{
	data_[key].exposed=exp;
}

void session_interface::hide(string const &key)
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




} // cppcms

