#ifndef CPPCMS_UTIL_H
#define CPPCMS_UTIL_H

#include "defs.h"
#include <string>

namespace cppcms {

	namespace util {
		std::string CPPCMS_API escape(std::string const &s);
		std::string CPPCMS_API urlencode(std::string const &s);
	}
}

#endif
