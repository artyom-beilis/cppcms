#ifndef CPPCMS_ERROR_H
#define CPPCMS_ERROR_H

#include "defs.h"
#include <string>
#include <stdexcept>

namespace cppcms {


class CPPCMS_API cppcms_error : public std::runtime_error {
	std::string strerror(int err);
public:
	cppcms_error(int err,std::string const &error);
	cppcms_error(std::string const &error) : std::runtime_error(error) {};
};

}
#endif /* _HTTP_ERROR_H */
