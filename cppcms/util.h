///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_UTIL_H
#define CPPCMS_UTIL_H

#include <cppcms/defs.h>
#include <string>

namespace cppcms {

	///
	/// \brief This namespace holds various useful helper functions for we developer
	///

	namespace util {
		///
		/// Escape string for inclusion in HTML page, i.e.
		///
		/// - < - \&lt;
		/// - > - \&gt;
		/// - \& - \&amp;
		/// - &quot; - \&quot;
		///
		/// Note, this function does not deal with encodings, so it's up to you to
		/// provide valid text encoding
		///
		std::string CPPCMS_API escape(std::string const &s);
		///
		/// Escape string for inclusion in HTML page, i.e.
		///
		/// - < - \&lt;
		/// - > - \&gt;
		/// - \& - \&amp;
		/// - &quot; - \&quot;
		///
		/// Note, this function does not deal with encodings, so it's up to you to
		/// provide valid text encoding
		///
		void CPPCMS_API escape(char const *begin,char const *end,std::ostream &output);
		///
		/// Encode string for URL (percent encoding)
		///
		std::string CPPCMS_API urlencode(std::string const &s);
		///
		/// Encode string for URL (percent encoding)
		///
		void CPPCMS_API urlencode(char const *begin,char const *end,std::ostream &output);
		///
		/// Decode string from URL-encoding (percent-encoding)
		///
		std::string CPPCMS_API urldecode(std::string const &s);
		///
		/// Decode text in range [begin,end) from URL-encoding (percent-encoding)
		///
		std::string CPPCMS_API urldecode(char const *begin,char const *end);
		///
		/// Make MD5 hash of string \a input converting into binary string of 16 bytes
		///
		std::string CPPCMS_API md5(std::string const &input);
		///
		/// Make MD5 hash of string \a input converting it into hexadecimal string representing this hash
		///
		std::string CPPCMS_API md5hex(std::string const &input);

	}
}

#endif
