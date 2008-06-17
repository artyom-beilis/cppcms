#ifndef CPPCMS_CGI_API_H
#define CPPCMS_CGI_API_H
#include "cgicc_connection.h"

namespace cppcms {

class cgi_session {
public:
	virtual cgicc_connection &get_connection() = 0;
	virtual bool prepare() = 0;
	virtual ~cgi_session() {};
};

class cgi_api {
public:
	virtual int get_socket() = 0;
	virtual cgi_session *accept_session() = 0;
	virtual ~cgi_api() {};
};

};

#endif
