///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_ERROR_H
#define CPPCMS_ERROR_H

#include <cppcms/defs.h>
#include <string>
#include <booster/backtrace.h>
namespace cppcms {

///
/// \brief Exception thrown by CppCMS framework.
///
/// Every exception that is thrown from CppCMS modules derived from this exception.
///

class CPPCMS_API cppcms_error : public booster::runtime_error {
	std::string strerror(int err);
public:
	///
	/// Create an object with error code err (errno) and a message \a error
	///
	cppcms_error(int err,std::string const &error);
	///
	/// Create an object with message \a error
	///
	cppcms_error(std::string const &error) : booster::runtime_error(error) {};
};

}
#endif 
