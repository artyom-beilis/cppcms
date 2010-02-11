#ifndef CPPCMS_UTIL_H
#define CPPCMS_UTIL_H

#include "defs.h"
#include <string>

namespace cppcms {

	namespace util {
		///
		/// Escape string for inclusion in HTML page, i.e.
		/// < --- &lt;
		/// > --- &gt;
		/// & --- &amp;
		/// " --- &quot;
		///
		/// Note, this function does not deal with encodings, so it's up to you to
		/// provide valid text encoding
		///
		std::string CPPCMS_API escape(std::string const &s);
		///
		/// Encode string for URL (percent encoding)
		///
		std::string CPPCMS_API urlencode(std::string const &s);
		///
		/// Decode string from URL-encoding (percent-encoding)
		///
		std::string CPPCMS_API urldecode(std::string const &s);
		///
		/// Decode text in range [begin,end) from URL-encoding (percent-encoding)
		///
		std::string CPPCMS_API urldecode(char const *begin,char const *end);
	}
}

#endif
