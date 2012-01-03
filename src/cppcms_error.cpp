///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/cppcms_error.h>
#include <cppcms/config.h>
#include <iostream>
#include <string.h>

#include <booster/system_error.h>

using namespace std;


namespace cppcms {

cppcms_error::cppcms_error(int err,std::string const &error) :
	booster::runtime_error(error+":" + strerror(err))	
{
}

std::string cppcms_error::strerror(int err)
{
	return booster::system::error_code(err,booster::system::system_category).message();
}

}
