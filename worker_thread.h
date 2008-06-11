#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <pthread.h>
#include <sstream>
#include <string>

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTTPStatusHeader.h"
#include "cgicc/HTMLClasses.h"
#include <memory>

#include "fcgi_stream.h"
#include "cppcms_error.h"
#include "url.h"
#include "cache_interface.h"
#include "base_cache.h"


namespace cppcms {

using namespace std;
using cgicc::CgiEnvironment;
using cgicc::Cgicc;
using cgicc::HTTPHeader;


class worker_thread {
friend class url_parser;
friend class cache_iface;
protected:	
	auto_ptr<fcgi_stream>io;
	auto_ptr<Cgicc> cgi;
	CgiEnvironment const *env;

	auto_ptr<HTTPHeader> response_header;
	void set_header(HTTPHeader*h){response_header=auto_ptr<HTTPHeader>(h);};
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

	void run(FCGX_Request *req);

	worker_thread() : cache(this) { init_internal(); } ;
	virtual ~worker_thread();
};

}

#endif
