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

#include "fcgi.h"
#include "cppcms_error.h"
#include "url.h"
#include "cache_interface.h"
#include "base_cache.h"
#include "cgicc_connection.h"

namespace cppcms {

using namespace std;
using cgicc::CgiEnvironment;
using cgicc::Cgicc;
using cgicc::HTTPHeader;


class worker_thread: private boost::noncopyable {
friend class url_parser;
friend class cache_iface;
	cache_factory const &cf;
protected:
	Cgicc *cgi;
	CgiEnvironment const *env;

	auto_ptr<HTTPHeader> response_header;
	list<string> other_headers;
	void set_header(HTTPHeader*h){response_header=auto_ptr<HTTPHeader>(h);};
	void add_header(string s) { other_headers.push_back(s); };
	virtual void main();

	// Output and Cahce

	cache_iface cache;
	base_cache *caching_module;
	bool gzip;
	bool gzip_done;
	string out;
	void init_internal();
public:
	int id;
	pthread_t pid;

	void run(cgicc_connection &);

	worker_thread(cache_factory const &f=cache_factory()) : cf(f), cache(this) { init_internal(); } ;
	virtual ~worker_thread();
};

}

#endif
