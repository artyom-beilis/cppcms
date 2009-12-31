#ifndef FCGI_STREAM_H
#define FCGI_STREAM_H

#include "config.h"

#include <string>

#ifdef EN_FASTCGI_LONG_PATH
	#include <fastcgi/fcgio.h>
	#include <fastcgi/fcgiapp.h>
#else
	#include <fcgio.h>
	#include <fcgiapp.h>
#endif

#include <cgicc/CgiInput.h>
#include <boost/noncopyable.hpp>
#include "cgi_api.h"
#include "cgicc_connection.h"

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
	virtual ~fcgi_stream();
};

class cgicc_connection_fast_cgi : public cgicc_connection {
	fcgi_stream fcgi;
	auto_ptr<cgicc::Cgicc> save_cgi;
	public:
	cgicc_connection_fast_cgi(FCGX_Request &req) :
		fcgi(req),
	save_cgi(new cgicc::Cgicc(&fcgi))
	{};
	virtual string env(char const *var){ return fcgi.getenv(var);};
	virtual cgicc::Cgicc &cgi() { return *save_cgi; };
	virtual ostream &cout() { return fcgi; };
	virtual ~cgicc_connection_fast_cgi() {};


};

class fcgi_session : public cgi_session {
	FCGX_Request *request;
	cgicc_connection_fast_cgi *connection;
public:
	fcgi_session(FCGX_Request *r,cgicc_connection_fast_cgi *conn) :
		request(r), connection(conn) {};
	virtual cgicc_connection &get_connection();
	virtual bool prepare();
	virtual ~fcgi_session() {
		delete connection;
		delete request;
	};
};

class fcgi_api : public cgi_api {
	static pthread_once_t init_fcgi;
	static void init(void);
	int fd;
	long long content_length_limit;
public:
	fcgi_api(char const *socket,int backlog,long long limit);
	virtual int get_socket() { return fd; };
	virtual cgi_session *accept_session();
	virtual ~fcgi_api();
};

} // Namespace
#endif
