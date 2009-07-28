#ifndef CPPCMS_UTIL_H
#define CPPCMS_UTIL_H
#include <string>
namespace cppcms {
std::string escape(std::string const &s);
std::string urlencode(std::string const &s);
namespace util {
	inline char ascii_tolower(char c)
	{
		if(c<'A' || c>'Z') return c;
		return c-'A'+'a';
	}
}


}

#endif
