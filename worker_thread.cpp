#include "worker_thread.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace cgicc;

Worker_Thread::Worker_Thread()
{
}

void Worker_Thread::main()
{
	out.puts("<h1>Hello World</h2>\n");
}


void Worker_Thread::run(FCGX_Request *fcgi)
{
#ifdef FCGX_API_ACCEPT_ONLY_EXISTS
	if(FCGX_Continue_r(fcgi)<0){
		return;
	}
#endif	
	
	io = auto_ptr<FCgiIO>(new FCgiIO(*fcgi));
	cgi = auto_ptr<Cgicc>( new Cgicc(&*io) );
	env=&(cgi->getEnvironment());

	set_header(new HTTPHTMLHeader);
	
	try {
		/**********/
		main();
		/**********/
		if(response_header.get() == NULL) {
			throw HTTP_Error("Looks like a bug");
		}
	}
	catch( HTTP_Error &error_message) {
		int err_code;
		out.reset();
		string msg;
		if(error_message.is_404()) {
			err_code=404;
			msg="Not Found: ";
			msg+=error_message.get();
		}
		else {
			err_code=500;
			msg=error_message.get();
		}
		set_header(new HTTPStatusHeader(err_code,msg));
		out.puts(msg.c_str());
	}

	char *ptr;
	
	bool gzip=false;

	if((ptr=FCGX_GetParam("HTTP_ACCEPT_ENCODING",fcgi->envp))!=NULL) {
		if(strstr(ptr,"gzip")!=NULL) {
			gzip=true;
		}
	}

	if(global_config.lval("gzip.enable",0)==0) {
		gzip=false;
	}
	
	if(gzip) {
		using namespace boost::iostreams;
		*io<<"Content-Encoding: gzip\r\n";
		*io<<*response_header;
		gzip_params params;
		long level,length;

		if((level=global_config.lval("gzip.level",-1))!=-1){
			params.level=level;
		}		

		filtering_ostream zstream;

		if((length=global_config.lval("gzip.buffer",-1))!=-1){
			zstream.push(gzip_compressor(params,length));
		}
		else {
			zstream.push(gzip_compressor(params));
		}

		zstream.push(*io);
		zstream<<out.get();
	}	
	else {
		*io<<*response_header;
		*io<<out.get();
	}
	
	out.reset();
	
	response_header.reset();
	cgi.reset();
	io.reset();
	
        FCGX_Finish_r(fcgi);
}
