#ifndef _HTTP_ERROR_H
#define _HTTP_ERROR_H

#include <string>
#include <stdexcept>

namespace cppcms {

class cppcms_error : public std::runtime_error {
public:
	cppcms_error(std::string const &error) : std::runtime_error(error) {};
};

}
#endif /* _HTTP_ERROR_H */
