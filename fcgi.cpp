#include "fcgi.h"
#include "cppcms_error.h"
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

namespace cppcms {

fcgi_stream::fcgi_stream(FCGX_Request &req) :
		std::ostream(&fcgi_cout),
		request(req),
		fcgi_cout(req.out),
		fcgi_cerr(req.err),
		stream_cerr(&fcgi_cerr)
{
};

std::string fcgi_stream::getenv(const char *variable)
{
	char const *p;
	if((p=FCGX_GetParam(variable,request.envp))!=NULL)
		return p;
	return "";
};

size_t fcgi_stream::read(char *d,size_t len)
{
	return FCGX_GetStr(d,len,request.in);
};

std::ostream &fcgi_stream::err()
{
	return stream_cerr;
};

fcgi_stream::~fcgi_stream()
{
	FCGX_Finish_r(&request);
};

pthread_once_t fcgi_api::init_fcgi = PTHREAD_ONCE_INIT;

void fcgi_api::init()
{
	FCGX_Init();
}

fcgi_api::fcgi_api(char const *socket,int backlog)
{
	pthread_once(&init_fcgi,fcgi_api::init);
	if(socket && socket[0]!='\0') {
		fd=FCGX_OpenSocket(socket,backlog);
	}
	else {
		fd=0; // STDIN
	}
	if(fd<0) {
		throw cppcms_error(errno,"FCGX_OpenSocket");
	}
}

cgi_session *fcgi_api::accept_session()
{
	FCGX_Request *request=new FCGX_Request();
	FCGX_InitRequest(request,fd,FCGI_FAIL_ACCEPT_ON_INTR);
	if(FCGX_Accept_r(request)<0) {
		delete request;
		return NULL;
	}
	return new fcgi_session(request,NULL);
}

bool fcgi_session::prepare()
{
	connection=new cgicc_connection_fast_cgi(*request);
	return true;
}

cgicc_connection &fcgi_session::get_connection()
{
	if(!connection) {
		throw cppcms_error("Connection was not prepared");
	}
	return *connection;
}

fcgi_api::~fcgi_api()
{
	if(fd!=-1) close(fd);
}


};
