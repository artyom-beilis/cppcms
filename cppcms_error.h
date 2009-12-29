#ifndef CPPCMS_ERROR_H
#define CPPCMS_ERROR_H

#include "defs.h"
#include <string>
#include <stdexcept>

namespace cppcms {

///
/// \brief Exception thrown by CppCMS framework.
///
/// Every exception that is thrown from CppCMS modules derived from this exception.
///

class CPPCMS_API cppcms_error : public std::runtime_error {
	std::string strerror(int err);
public:
	///
	/// Create an object with error code err (errno) and a message \a error
	///
	cppcms_error(int err,std::string const &error);
	///
	/// Create an object with message \a error
	///
	cppcms_error(std::string const &error) : std::runtime_error(error) {};
};

}
#endif /* _HTTP_ERROR_H */
