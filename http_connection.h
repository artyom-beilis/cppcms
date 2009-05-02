#ifndef CPPCMS_HTTP_CONNECTION_H
#define CPPCMS_HTTP_CONNECTION_H

namespace cppcms { namespace http {

class connection {
public:
	virtual std::string getenv(std::string const &key) = 0;
	virtual size_t input(char *buffer,size_t size) = 0;
	virtual std::ostream &output() = 0;
	virtual ~connection(){}
};


} } // cppcms::http

#endif

