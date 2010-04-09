///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <pthread.h>
#include <sstream>
#include <string>

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTTPStatusHeader.h>
#include <cgicc/HTMLClasses.h>
#include "noncopyable.h"
#include <memory>
#include "signal0.h"

#include "cppcms_error.h"
#include "url_dispatcher.h"
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

class worker_thread: private util::noncopyable {
	int id;
	pthread_t pid;

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

	url_dispatcher url;
	manager const &app;
	Cgicc *cgi;
	CgiEnvironment const *env;
	cgicc_connection *cgi_conn;

	cache_iface cache;
	ostream cout;
	util::signal0 on_start;
	util::signal0 on_end;
	session_interface session;

	void set_header(HTTPHeader *h);
	void add_header(string s);
	void set_cookie(cgicc::HTTPCookie const &c);
	void set_user_io();
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
