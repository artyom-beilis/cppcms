#include "worker_thread.h"
#include "session_interface.h"
#include "session_api.h"
#include "manager.h"
#include "archive.h"

namespace cppcms {

session_interface::session_interface(worker_thread &w) :
	worker(w)
{
	timeout_val_def=w.app.config.ival("session.timeout",24*3600);
	string s_how=w.app.config.sval("session.expire","browser");
	if(s_how=="fixed") {
		how_def=fixed;
	}
	else if(s_how=="renew") {
		how_def=renew;
	}
	else if(s_how=="browser") {
		how_def=browser;
	}
	else {
		throw cppcms_error("Unsupported `session.expire' type `"+s_how+"'");
	}


	storage=w.app.sessions(w);
}

worker_thread &session_interface::get_worker()
{
	return worker;
}

void session_interface::set_api(boost::shared_ptr<session_api> s)
{
	storage=s;
}
bool session_interface::load()
{
	data.clear();
	data_copy.clear();
	timeout_val=timeout_val_def;
	how=how_def;
	archive ar;
	if(!storage.get() || !storage->load(this,ar.set(),timeout_in)) {
		return false;
	}
	int i,num;
	ar>>num;
	for(i=0;i<num;i++) {
		string key,val;
		ar>>key>>val;
		data[key].swap(val);
	}
	data_copy=data;
	return true;
}

int session_interface::cookie_age()
{
	if(how==browser)
		return 0;
	if(how==renew || ( how==fixed && new_session ))
		return timeout_val;
	return timeout_in - time(NULL);
}

time_t session_interface::session_age()
{
	if(how==browser || how==renew || (how==fixed && new_session))
		return timeout_val + time(NULL);
	return timeout_in;
}

void session_interface::save()
{
	check();
	new_session  = data_copy.empty() && !data.empty();
	if(data.empty()) {
		if(get_session_cookie()!="")
			storage->clear(this);
		return;
	}

	time_t now = time(NULL);

	if(data==data_copy) {
		if(how==fixed) {
			return;
		}
		if(how==renew || how==browser) {
			int64_t delta=now + timeout_val - timeout_in;
			if(delta < timeout_val * 0.1) {// Less then 10% -- no renew need
				return;
			}
		}
	}
	int num=data.size();
	archive ar;
	ar<<num;
	for(map<string,string>::iterator p=data.begin(),e=data.end();p!=e;++p) {
		ar<<p->first<<p->second;
	}
	temp_cookie.clear();
	storage->save(this,ar.get(),session_age(),new_session);
	set_session_cookie(cookie_age(),temp_cookie);
	temp_cookie.clear();
}

void session_interface::on_start()
{
	load();
}

void session_interface::on_end()
{
	if(storage.get()!=NULL)
		save();
}

void session_interface::check()
{
	if(storage.get()==NULL)
		throw cppcms_error("Session storage backend is not loaded\n");
}

string &session_interface::operator[](string const &key)
{
	return data[key];
}

void session_interface::del(string const &key)
{
	data.erase(key);
}

bool session_interface::is_set(string const &key)
{
	return data.find(key)!=data.end();
}

void session_interface::clear()
{
	data.clear();
}

void session_interface::clear_session_cookie()
{
	if(get_session_cookie()!="")
		set_session_cookie(-1,"");
}
void session_interface::set_session_cookie(int64_t age,string const &data)
{
	if(data.empty())
		age=0;
	cgicc::HTTPCookie
		cookie(	worker.app.config.sval("session.cookies_prefix","cppcms_session"), // name
			(age >= 0 ? data : ""), // value
			"",   // comment
			worker.app.config.sval("session.cookies_domain",""), // domain
			( age < 0 ? 0 : age ),
			worker.app.config.sval("session.cookies_path","/"),
			worker.app.config.ival("session.cookies_secure",0));
	worker.set_cookie(cookie);
}
void session_interface::set_session_cookie(string const &data)
{
	temp_cookie=data;
}

string session_interface::get_session_cookie()
{
	string name=worker.app.config.sval("session.cookies_prefix","cppcms_session");
	vector<cgicc::HTTPCookie> const &cookies=worker.env->getCookieList();
	for(unsigned i=0;i<cookies.size();i++) {
		if(cookies[i].getName()==name)
			return cookies[i].getValue();
	}
	return string("");
}
	
void session_interface::get(std::string const &key,serializable &s)
{
	s=(*this)[key];
}
void session_interface::set(std::string const &key,serializable const &s)
{
	(*this)[key]=s;
}


};

