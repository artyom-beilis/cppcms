#ifndef FCGI_STREAM_H
#define FCGI_STREAM_H

#include <string>
#include <iostream>
#include <fcgio.h>
#include <cgicc/CgiInput.h>
#include <boost/noncopyable.hpp>

namespace cppcms {

class fcgi_stream :	public cgicc::CgiInput,
			public std::ostream,
			private boost::noncopyable
{
	FCGX_Request &request;
	fcgi_streambuf fcgi_cout;
	fcgi_streambuf fcgi_cerr;
	std::ostream   stream_cerr;
public:
	fcgi_stream(FCGX_Request &req); 
	virtual std::string getenv(const char *variable);
	virtual size_t read(char *d,size_t len);
	std::ostream &err();
};

} // Namespace
#endif
