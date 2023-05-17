//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_SYSTEM_ERROR_H
#define BOOSTER_SYSTEM_ERROR_H

#include <string>
#include <booster/backtrace.h>
#include <functional>

#include <booster/config.h>
#include <system_error>
namespace booster {

///
/// \brief this namespace includes partial implementation of std::tr1's/boost's system_error, error_code
/// classes 
///
namespace system {
	using std::error_category;
	using std::system_category;
	using std::error_code;

    class system_error : public std::system_error, public backtrace  {
    public:
        using std::system_error::system_error;
    };

} // system
} // booster


#endif
