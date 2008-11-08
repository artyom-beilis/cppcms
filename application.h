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


