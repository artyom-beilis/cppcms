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
#include <boost/signal.hpp>

#include "cppcms_error.h"
#include "url.h"
#include "cache_interface.h"
#include "base_cache.h"
#include "cgicc_connection.h"
#include "transtext.h"
#include "session_interface.h"

namespace cppcms {

class manager;
class base_content;

using namespace std;
using cgicc::CgiEnvironment;
using cgicc::Cgicc;
using cgicc::HTTPHeader;

class worker_thread: private boost::noncopyable {
	int id;
	pthread_t pid;

	friend class url_parser;
	friend class cache_iface;
	friend class base_view;

	list<string> other_headers;
	base_cache *caching_module;
	bool user_io;
	bool gzip;
	bool gzip_done;
	stringbuf out_buf;

	transtext::trans const *gt;
	string lang;

	auto_ptr<HTTPHeader> response_header;
	string current_template;

public:

	url_parser url;
	manager const &app;
	Cgicc *cgi;
	CgiEnvironment const *env;
	cgicc_connection *cgi_conn;

	cache_iface cache;
	ostream cout;
	boost::signal<void()> on_start;
	boost::signal<void()> on_end;
	session_interface session;

	void set_header(HTTPHeader *h);
	void add_header(string s);
	void set_cookie(cgicc::HTTPCookie const &c);
	void set_user_io();
	void flush_headers();
	void no_gzip();

	HTTPHeader &header();

	void set_lang();
	void set_lang(string const &s);

	inline void use_template(string s="") { current_template=s; };

	void render(string name,base_content &content);
	void render(string templ,string name,base_content &content);
	void render(string name,base_content &content,ostream &);
	void render(string templ,string name,base_content &content,ostream &);

	virtual void main();

	inline char const *gettext(char const *s) { return gt->gettext(s); };
	inline char const *ngettext(char const *s,char const *p,int n) { return gt->ngettext(s,p,n); };

	ostream &get_cout() { return cout; }

	transtext::trans const *domain_gettext(string const &domain);

	void run(cgicc_connection &);

	worker_thread(manager const &s);
	virtual ~worker_thread();
};

}

#endif
