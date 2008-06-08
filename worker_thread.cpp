#include "worker_thread.h"
#include "global_config.h"
#include "thread_cache.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace cgicc;
namespace cppcms {

void worker_thread::main()
{
	out="<h1>Hello World</h2>\n";
}


void worker_thread::run(FCGX_Request *fcgi)
{
	io = auto_ptr<FCgiIO>(new FCgiIO(*fcgi));
	cgi = auto_ptr<Cgicc>(new Cgicc(&*io));
	env=&(cgi->getEnvironment());

	out.clear();
	out.reserve(global_config.lval("performance.textalloc",65500));
	cache.reset();

	set_header(new HTTPHTMLHeader);

	gzip=gzip_done=false;
	char *ptr;
	if((ptr=FCGX_GetParam("HTTP_ACCEPT_ENCODING",fcgi->envp))!=NULL) {
		if(strstr(ptr,"gzip")!=NULL) {
			gzip=global_config.lval("gzip.enable",0);
		}
	}

	try {
		/**********/
		main();
		/**********/
		if(response_header.get() == NULL) {
			throw cppcms_error("Looks like a bug");
		}
	}
	catch(cppcms_error const &e) {
		string msg=e.what();
		set_header(new HTTPStatusHeader(500,msg));
		out="<html><body><p>"+msg+"</p><body></html>";
	}


	if(gzip) {
		*io<<"Content-Encoding: gzip\r\n";
		*io<<*response_header;
		if(gzip_done)
			*io<<out;
		else
			deflate(out,*io);
	}	
	else {
		*io<<*response_header;
		*io<<out;
	}

	// Clean Up
	out.clear();
	response_header.reset();
	cgi.reset();
	io.reset();
        FCGX_Finish_r(fcgi);
}

void worker_thread::init_internal()
{
	string backend=global_config.sval("cache.backend","none");
	caching_module=NULL;
	if(backend=="threaded") {
		static thread_cache tc;
		tc.set_size(global_config.lval("cache.limit",100));
		caching_module=&tc;
		if(global_config.lval("cache.debug",0)==1) {
			tc.set_debug_mode(2);
		}
	}
}

worker_thread::~worker_thread()
{
}



}



