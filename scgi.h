#ifndef CPPCMS_SCGI_H
#define CPPCMS_SCGI_H

#include <cgicc/CgiInput.h>
#include <boost/noncopyable.hpp>
#include <ostream>
#include <streambuf>
#include <map>
#include <unistd.h>
#include "cppcms_error.h"

namespace cppcms {
using namespace std;

namespace scgi {
	class scgi_outbuffer : public streambuf
	{
		int fd;
	public:
		scgi_outbuffer(int descriptor) : fd(descriptor) {};
		virtual streamsize xsputn ( const char * s, streamsize n );
		virtual ~scgi_outbuffer();
	};

};

class scgi_connection : public cgicc::CgiInput,
			private boost::noncopyable,
			public std::ostream
{
	int socket;
	scgi::scgi_outbuffer buf;
	map<string,string> env;

public:
	scgi_connection(int s) :std::ostream(&buf), socket(s), buf(s) {};
	bool prepare();
	virtual size_t 	read(char *data, size_t length);
	virtual std::string getenv(const char *var);
	virtual ~scgi_connection();
};

class scgi {
	int fd;
	void throwerror(char const *s);
public:
	scgi(string socket,int backlog=1);
	~scgi();
	inline int socket() { return fd; };
	int accept();
};


};


#endif
