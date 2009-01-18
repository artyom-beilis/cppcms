#include "worker_thread.h"
#include "session_interface.h"
#include "session_api.h"
#include "manager.h"
#include "util.h"
#include <sstream>

namespace cppcms {

class cookie : public cgicc::HTTPCookie {
	bool del;
public:
	cookie() :
	 del(false)
	{
	}
	cookie(string const &name,string const &val) :
		cgicc::HTTPCookie(name,val),
		del(false)

	{
	}
	cookie(	const std::string& name,
		const std::string& value,
		const std::string& comment,
		const std::string& domain,
		unsigned long maxAge,
		const std::string& path,
		bool secure) :
			HTTPCookie(name,value,comment,domain,maxAge,path,secure),
			del(false)
	{
	}
	void remove() { del=true; }
	virtual void render(std::ostream& out) const
	{
		if(!del) {
			cgicc::HTTPCookie::render(out);
		}
		else {
			out <<"Set-Cookie:"<<getName()<<"=";
			string domain=getDomain();
			if(!domain.empty()) {
				out<<"; Domain="<<domain;
			}
			string path=getPath();
			if(!path.empty()) {
				out<<"; Path="<<path;
			}
			if(isSecure()) {
				cout<<"; Secure";
			}
			out<<"; Expires=Fri, 01-Jan-1971 01:00:00 GMT; Version=1";
		}
	}
};


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
	string ar;
	if(!storage.get() || !storage->load(this,ar,timeout_in)) {
		return false;
	}
	load_data(data,ar);
	data_copy=data;
	if(is_set("_t"))
		timeout_val=get<int>("_t");
	if(is_set("_h"))
		how=get<int>("_h");
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

namespace {
struct packed {
        unsigned key_size  : 10;
        unsigned exposed   :  1;
        unsigned data_size : 21;
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


void session_interface::save_data(data_t const &data,std::string &s)
{
	s.clear();
	data_t::const_iterator p;
	for(p=data.begin();p!=data.end();++p) {
		packed header(p->first.size(),p->second.exposed,p->second.value.size());
		char *ptr=(char *)&header;
		s.append(ptr,ptr+sizeof(header));
		s.append(p->first.begin(),p->first.end());
		s.append(p->second.value.begin(),p->second.value.end());
	}
}

void session_interface::load_data(data_t &data,std::string const &s)
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



void session_interface::update_exposed()
{
	for(data_t::iterator p=data.begin();p!=data.end();++p) {
		data_t::iterator p2=data_copy.find(p->first);
		if(p->second.exposed && (p2==data_copy.end() || !p2->second.exposed || p->second.value!=p2->second.value)){
			set_session_cookie(cookie_age(),p->second.value,p->first);
		}
		else if(!p->second.exposed && p2!=data_copy.end() && p2->second.exposed) {
			set_session_cookie(-1,"",p->first);
		}
	}
	for(data_t::iterator p=data_copy.begin();p!=data_copy.end();++p) {
		if(p->second.exposed && data.find(p->first)==data.end()) {
			set_session_cookie(-1,"",p->first);
		}
	}
}

void session_interface::save()
{
	check();
	new_session  = data_copy.empty() && !data.empty();
	if(data.empty()) {
		if(get_session_cookie()!="")
			storage->clear(this);
		update_exposed();
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

	string ar;
	save_data(data,ar);
	
	temp_cookie.clear();
	storage->save(this,ar,session_age(),new_session);
	set_session_cookie(cookie_age(),temp_cookie);
	temp_cookie.clear();

	update_exposed();	
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
	return data[key].value;
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
void session_interface::set_session_cookie(int64_t age,string const &data,string const &key)
{
	if(data.empty())
		age=-1;
	string cookie_name=worker.app.config.sval("session.cookies_prefix","cppcms_session");
	if(!key.empty()) {
		cookie_name+="_";
		cookie_name+=key;
	}
	if(age < 0) {
		cookie cook(cookie_name,"","",
			worker.app.config.sval("session.cookies_domain",""),
			0,
			worker.app.config.sval("session.cookies_path","/"),
			worker.app.config.ival("session.cookies_secure",0));
		cook.remove();
		ostringstream out;
		out<<cook;
		worker.add_header(out.str());
	}
	else {
		cgicc::HTTPCookie cook(
			cookie_name, // name
			(age >= 0 ? urlencode(data) : ""), // value
			"",   // comment
			worker.app.config.sval("session.cookies_domain",""), // domain
			( age < 0 ? 0 : age ),
			worker.app.config.sval("session.cookies_path","/"),
			worker.app.config.ival("session.cookies_secure",0));
		worker.set_cookie(cook);
	}

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

bool session_interface::is_exposed(std::string const &key)
{
	data_t::iterator p=data.find(key);
	if(p!=data.end()) 
		return p->second.exposed;
	return false;
}

void session_interface::expose(string const &key,bool exp)
{
	data[key].exposed=exp;
}

void session_interface::hide(string const &key)
{
	expose(key,false);
}

void session_interface::set_age(int t)
{
	timeout_val=t;
	set("_t",t);
}

void session_interface::set_age()
{
	del("_t");
	timeout_val=timeout_val_def;
}

void session_interface::set_expiration(int h)
{
	how=h;
	set("_h",h);
}
void session_interface::set_expiration()
{
	del("_h");
	how=how_def;
}





};

