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
#include "cppcms_error.h"
#include "config.h"
#include <iostream>
#include <string.h>

#ifndef HAVE_STRERROR_R
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/system/error_code.hpp>
#else // Internal Boost
#   include <cppcms_boost/system/error_code.hpp>
    namespace boost = cppcms_boost;
#endif
#endif

using namespace std;


namespace cppcms {

cppcms_error::cppcms_error(int err,std::string const &error) :
	std::runtime_error(error+":" + strerror(err))	
{
}

// Unfortunatly I can't use XSI-compliant strerror_r() under g++
// it always gives GNU strerror_r, thus it is wrapped

namespace {
	string strerror_wrapper(int value,char *buf)
	{
		return buf;
	}
	string strerror_wrapper(char const *err,char *buf)
	{
		return err;
	}
}

std::string cppcms_error::strerror(int err)
{
	#ifdef HAVE_STRERROR_R
	char buf[256] = {0};
	return strerror_wrapper(strerror_r(err,buf,sizeof(buf)),buf);
	#else
	return boost::system::error_code(err,boost::system::errno_ecat).message();
	#endif
}

}
