#ifndef _HTTP_ERROR_H
#define _HTTP_ERROR_H

#include <string>
#include <stdexcept>
#include <string.h>

namespace cppcms {


class cppcms_error : public std::runtime_error {
	std::string strerror(int err)
	{
		char buf[256];
		strerror_r(err,buf,sizeof(buf));
		return buf;
	}
public:
	cppcms_error(int err,std::string const &error):
		std::runtime_error(error+":" + strerror(err)) {};
	cppcms_error(std::string const &error) : std::runtime_error(error) {};
};

}
#endif /* _HTTP_ERROR_H */
