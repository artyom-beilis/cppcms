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
#define CPPCMS_SOURCE
#include "regex.h"
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/regex.hpp>
#else // Internal Boost
#   include <cppcms_boost/regex.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms { namespace util {

	struct regex_result::data {
		std::string str;
		boost::cmatch match;
	};
	regex_result::regex_result() : d(new data)
	{
	}
	regex_result::~regex_result()
	{
	}
	std::string regex_result::operator[](int n)
	{
		return d->match[n];
	}

	struct regex::data {
		boost::regex r;
		data(std::string const &e) : r(e) {}
	};

	regex::regex(std::string const &e) : d(new data(e))
	{
	}
	regex::~regex()
	{
	}
	bool regex::match(std::string const &str,regex_result &res) const
	{
		//
		// Make sure that cmatch is valid, even if original string already is not
		// 
		res.d->str = str;
		return boost::regex_match(res.d->str.c_str(),res.d->match,d->r);
	}
	bool regex::match(std::string const &str) const
	{
		return boost::regex_match(str.c_str(),d->r);
	}


}} // cppcms::util
