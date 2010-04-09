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
#ifndef CPPCMS_UTIL_REGEX_H
#define CPPCMS_UTIL_REGEX_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"
#include <string>

namespace cppcms { namespace util {

	class regex_result;

	///
	/// \brief This class is used for matching regular expressions;
	///
	/// This class is actually used as a simple wrapper of \a boost::regex,
	/// \a smatch and \a match_results.
	///
	/// It is created in order to provide stable ABI and not depend on specific
	/// version of Boost. It mostly used as a simple API for regular expressions.
	///
	
	
	class CPPCMS_API regex : public noncopyable {
	public:
		///
		/// Creates new object that is used to match a regular \a expression.
		/// It is actually wrapper of boost::regex.
		///
		
		regex(std::string const &expression);

		~regex();

		///
		/// This function checks if string \a str matches the regular expression.
		/// If so, it stores the result in \a res and returns true, otherwise returs
		/// false. It actually calls \a boost::regex::match_result
		/// 
		bool match(std::string const &str,regex_result &res) const;
		///
		/// This function checks if string \a str matches the regular expression.
		/// If so, it returns true, otherwise returs false
		/// 
		bool match(std::string const &str) const;

	private:
		struct data;
		hold_ptr<data> d;
	};

	///
	/// \brief This class holds result of matching regular expression \a regex.
	/// 
	/// This class is actually wrapper of \a boost::regex::smatch and used entirely with
	/// \a util::regex.
	///

	class CPPCMS_API regex_result : public noncopyable {
	public:
		regex_result();
		~regex_result();
		///
		/// This operator returns matched expression at position \a n
		///
		std::string operator[](int n);
	private:
		friend class regex;
		struct data;
		hold_ptr<data> d;
	};

}} // cppcms::util


#endif
