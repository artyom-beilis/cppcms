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
#ifndef CPPCMS_UTIL_H
#define CPPCMS_UTIL_H

#include <cppcms/defs.h>
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
		///
		/// Make MD5 hash of string
		///
		std::string CPPCMS_API md5(std::string const &input);
		///
		/// Make MD5 hash of string as hexadecimal string
		///
		std::string CPPCMS_API md5hex(std::string const &input);

	}
}

#endif
