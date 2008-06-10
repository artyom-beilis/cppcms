#include "fcgi_stream.h"

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

};
