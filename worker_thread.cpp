#include "worker_thread.h"


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
	
	*io<<*response_header;
	*io<<out.get();
	
	out.reset();
	
	response_header.reset();
	cgi.reset();
	io.reset();
	
        FCGX_Finish_r(fcgi);
}
