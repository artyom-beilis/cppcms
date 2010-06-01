///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_ENCODING_H
#define CPPCMS_ENCODING_H

#include <string>
#include <map>
#include <locale>
#include <cppcms/defs.h>
#include <cppcms/config.h>

namespace cppcms {
	///
	/// \brief this Namespace holds various function for dealing with encoding
	///
	///
	namespace encoding {

		/// Note: all these function assume that control characters that invalid in HTML are illegal.
		/// For example. NUL is legal UTF-8 code but it is illegal in terms of HTML validity thus,
		/// valid_utf8 would return false.
		
		///
		/// Check if string in range [begin,end) is valid in the locale \a loc and does not include
		/// HTML illegal characters. Number of codepoints is stored in \a count
		///
		bool CPPCMS_API valid(std::locale const &loc,char const *begin,char const *end,size_t &count);
		///
		/// Check if string in range [begin,end) is valid UTF-8 and does not include
		/// HTML illegal characters. Number of codepoints is stored in \a count
		///
		bool CPPCMS_API valid_utf8(char const *begin,char const *end,size_t &count);
		///
		/// Check if string in range [begin,end) is valid encoding \a encoding and does not include
		/// HTML illegal characters. Number of codepoints is stored in \a count
		///
		bool CPPCMS_API valid(char const *encoding,char const *begin,char const *end,size_t &count);
		///
		/// Check if string in range [begin,end) is valid encoding \a encoding and does not include
		/// HTML illegal characters. Number of codepoints is stored in \a count
		///
		bool CPPCMS_API valid(std::string const &encoding,char const *begin,char const *end,size_t &count);

		#if defined(CPPCMS_HAVE_ICU) || defined(CPPCMS_HAVE_ICONV)

		///
		/// Convert string in range [begin,end) from local 8 bit encoding according to locale \a loc to UTF-8
		/// If illegal characters found, the conversion is aborted and only sucessefully converted part is returned.
		///
		std::string CPPCMS_API to_utf8(std::locale const &loc,char const *begin,char const *end);
		///
		/// Convert string in range [begin,end) from local 8 bit encoding \a encoding to UTF-8
		/// If illegal characters found, the conversion is aborted and only sucessefully converted part is returned.
		///
		std::string CPPCMS_API to_utf8(char const *encoding,char const *begin,char const *end);
		///
		/// Convert string \a str from local 8 bit encoding according to locale \a loc to UTF-8
		/// If illegal characters found, the conversion is aborted and only sucessefully converted part is returned.
		///
		std::string CPPCMS_API to_utf8(std::locale const &loc,std::string const &str);
		///
		/// Convert string \a str from local 8 bit encoding according to encoding \a encoding
		/// If illegal characters found, the conversion is aborted and only sucessefully converted part is returned.
		///
		std::string CPPCMS_API to_utf8(char const *encoding,std::string const &str);

		///
		/// Convert UTF-8 string in range [begin,end) to local 8 bit encoding according to locale \a loc.
		/// If non-convertable characters found, the conversion is aborted and only sucessefully converted part is returned.
		///
		std::string CPPCMS_API from_utf8(std::locale const &loc,char const *begin,char const *end);
		///
		/// Convert UTF-8 string in range [begin,end) to local 8 bit encoding \a encoding.
		/// If non-convertable characters found, the conversion is aborted and only sucessefully converted part is returned.
		///
		std::string CPPCMS_API from_utf8(char const *encoding,char const *begin,char const *end);
		///
		/// Convert UTF-8 string \a str to local 8 bit encoding according to locale \a loc.
		/// If non-convertable characters found, the conversion is aborted and only sucessefully converted part is returned.
		///
		std::string CPPCMS_API from_utf8(std::locale const &loc,std::string const &str);
		///
		/// Convert UTF-8 string \a str to local 8 bit encoding \a encoding.
		/// If non-convertable characters found, the conversion is aborted and only sucessefully converted part is returned.
		///
		std::string CPPCMS_API from_utf8(char const *encoding,std::string const &str);

		#endif

	}  // encoding
} // cppcms


#endif
