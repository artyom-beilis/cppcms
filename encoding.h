#ifndef CPPCMS_ENCODING_H
#define CPPCMS_ENCODING_H

#include <string>
#include <map>
#include <locale>
#include "defs.h"

namespace cppcms {
	namespace encoding {

		bool CPPCMS_API valid(std::locale const &loc,char const *begin,char const *end,size_t &count);
		bool CPPCMS_API valid_utf8(char const *begin,char const *end,size_t &count);
		bool CPPCMS_API valid(char const *encoding,char const *begin,char const *end,size_t &count);
		bool CPPCMS_API valid(std::string const &encoding,char const *begin,char const *end,size_t &count);

		std::string CPPCMS_API to_utf8(std::locale const &loc,char const *begin,char const *end);
		std::string CPPCMS_API to_utf8(char const *encoding,char const *begin,char const *end);
		std::string CPPCMS_API to_utf8(std::locale const &loc,std::string const &str);
		std::string CPPCMS_API to_utf8(char const *encoding,std::string const &str);

		std::string CPPCMS_API from_utf8(std::locale const &loc,char const *begin,char const *end);
		std::string CPPCMS_API from_utf8(char const *encoding,char const *begin,char const *end);
		std::string CPPCMS_API from_utf8(std::locale const &loc,std::string const &str);
		std::string CPPCMS_API from_utf8(char const *encoding,std::string const &str);

	}  // encoding
} // cppcms


#endif
