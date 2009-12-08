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
