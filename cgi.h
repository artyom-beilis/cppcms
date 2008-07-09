#ifndef CPPCMS_CGI_H
#define CPPCMS_CGI_H

namespace cppcms {

class cgi_cgi_session : public cgi_session {
	cgicc_connection_cgi cgi;
public:
	virtual cgicc_connection &get_connection() { return cgi; };
	virtual bool prepare() { return true; } ;
	virtual ~cgi_cgi_session() {};
};

class cgi_cgi_api : public cgi_api {
public:
	virtual int get_socket() { return 0; };
	virtual cgi_session *accept_session() { return new cgi_cgi_session(); };
	virtual ~cgi_cgi_api() {};
};

};

#endif
