#ifndef CPPCMS_APPLICATION_H
#define CPPCMS_APPLICATION_H
#include "worker_thread.h"
#include "manager.h"

namespace cppcms {

struct application {

	// Data
	worker_thread &worker;
	url_parser &url;
	manager const &app;
	Cgicc *&cgi;
	CgiEnvironment const *&env;
	cgicc_connection *&cgi_conn;

	cache_iface &cache;
	session_interface &session;

	ostream &cout;

	boost::signal<void()> &on_start;
	boost::signal<void()> &on_end;

	// Construction
	application(worker_thread &w);
	virtual ~application();
	// API

	void set_header(HTTPHeader *h) { worker.set_header(h); }
	void add_header(string s) { worker.add_header(s); }
	void set_cookie(cgicc::HTTPCookie const &c) { worker.set_cookie(c); }
	void set_user_io() { worker.set_user_io(); }

	HTTPHeader &header() { return worker.header(); }

	void set_lang() { worker.set_lang(); }
	void set_lang(string const &s) { worker.set_lang(s) ; }

	void use_template(string s="") { worker.use_template(s); }

	void render(string n,base_content &c) { worker.render(n,c); }
	void render(string t,string n,base_content &c) { worker.render(t,n,c); }
	void render(string n,base_content &c,ostream &o) { worker.render(n,c,o); }
	void render(string t,string n,base_content &c,ostream &o) { worker.render(t,n,c,o); }

	virtual void on_404();

	inline char const *gettext(char const *s) { return worker.gettext(s); };
	inline char const *ngettext(char const *s,char const *p,int n) { return worker.ngettext(s,p,n); }

	transtext::trans const *domain_gettext(string const &d) { return worker.domain_gettext(d); }

	virtual void main();

	template<typename SQL>
	void dbixx_load(SQL &sql)
	{
		using std::string;
		string driver=app.config.sval("dbixx.driver");
		sql.driver(driver);

		cppcms_config::range_t range=app.config.prefix(driver);
		size_t len=driver.size()+1;
		for(cppcms_config::data_t::const_iterator p=range.first;p!=range.second;++p){
			string param=p->first.substr(len);
			if(p->second.type()==typeid(string)) {
				string val=boost::any_cast<string>(p->second);
				sql.param(param,val);
			}
			else if(p->second.type()==typeid(int)) {
				int val=boost::any_cast<int>(p->second);
				sql.param(param,val);
			}
		}
		sql.connect();
	}

	template<typename SQL>
	void soci_load(SQL &sql)
	{
		string tmp;
		if(!(tmp=app.config.sval("soci.conn","")).empty()) {
			sql.open(app.config.sval("soci.conn"));
		}
		else {
			sql.open(app.config.sval("soci.driver"),app.config.sval("soci.params"));
		}
	}

};

template<typename T>
class application_worker : public worker_thread {
	T app;
public:
	application_worker(manager const &m) :
		worker_thread(m),
		app(*this)
	{
	}
	virtual void main()
	{
		app.main();
	}
};

template<typename T>
class application_factory : public simple_factory<application_worker<T> >
{
};

}

#endif


