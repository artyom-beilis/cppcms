#ifndef CPPCMS_CGICC_CONN_H
#define CPPCMS_CGICC_CONN_H

#include <boost/noncopyable.hpp>
#include "fcgi_stream.h"
#include <cgicc/Cgicc.h>

namespace cppcms {

class cgicc_connection : private boost::noncopyable {
public:
	virtual string env(char const *variable) = 0;
	virtual cgicc::Cgicc &cgi() = 0;
	virtual ostream &cout() = 0;
	virtual ~cgicc_connection() {};
};

class cgicc_connection_cgi : public cgicc_connection {
	auto_ptr<cgicc::Cgicc> save_cgi;
public:
	cgicc_connection_cgi() : save_cgi(new cgicc::Cgicc()) {};
	virtual string env(char const *var)
	{ 
		char const *ptr=getenv(var);
		if(ptr) return ptr;
		return "";
	};
	virtual cgicc::Cgicc &cgi() { return *save_cgi; };
	virtual ostream &cout() { return std::cout; };
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

};

#endif
