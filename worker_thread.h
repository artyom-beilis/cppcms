#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <pthread.h>
#include <sstream>
#include <string>

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTTPStatusHeader.h>
#include <cgicc/HTMLClasses.h>
#include <boost/noncopyable.hpp>
#include <memory>

#include "cppcms_error.h"
#include "url.h"
#include "cache_interface.h"
#include "base_cache.h"
#include "cgicc_connection.h"
#include "transtext.h"

namespace cppcms {

class manager;
class base_content;

using namespace std;
using cgicc::CgiEnvironment;
using cgicc::Cgicc;
using cgicc::HTTPHeader;

class worker_thread: private boost::noncopyable {
	friend class url_parser;
	friend class cache_iface;
	friend class base_view;

	list<string> other_headers;
	base_cache *caching_module;
	bool gzip;
	bool gzip_done;
	stringbuf out_buf;

	transtext::trans const *gt;
	string lang;
	
	auto_ptr<HTTPHeader> response_header;

protected:
	

	url_parser url;
	manager const &app;
	Cgicc *cgi;
	CgiEnvironment const *env;

	cache_iface cache;
	ostream cout;

	void set_header(HTTPHeader *h);
	void add_header(string s);
	void set_cookie(cgicc::HTTPCookie const &c);

	HTTPHeader &header();

	void set_lang();
	void set_lang(string const &s);

	inline char const *gettext(char const *s) { return gt->gettext(s); };
	inline char const *ngettext(char const *s,char const *p,int n) { return gt->ngettext(s,p,n); };


	string current_template;

	inline void use_template(string s="") { current_template=s; };

	void render(string name,base_content &content);
	void render(string templ,string name,base_content &content);
	void render(string name,base_content &content,ostream &);
	void render(string templ,string name,base_content &content,ostream &);
	virtual void main();
public:
	int id;
	pthread_t pid;
	ostream &get_cout() { return cout; }
	
	transtext::trans const *domain_gettext(string const &domain);

	void run(cgicc_connection &);

	worker_thread(manager const &s);
	virtual ~worker_thread();
};

}

#endif
