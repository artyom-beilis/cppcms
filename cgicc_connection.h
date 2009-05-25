#ifndef CPPCMS_CGICC_CONN_H
#define CPPCMS_CGICC_CONN_H

#include "noncopyable.h"
#include <cgicc/Cgicc.h>
#include <string>
#include <ostream>
#include <memory>

namespace cppcms {
using namespace std;

class cgicc_connection : private util::noncopyable {
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

};

#endif
