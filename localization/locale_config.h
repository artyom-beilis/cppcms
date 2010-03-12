//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_CONFIG_HPP_INCLUDED
#define CPPCMS_LOCALE_CONFIG_HPP_INCLUDED

#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/config.hpp>
#else // Internal Boost
#   include <cppcms_boost/config.hpp>
    namespace boost = cppcms_boost;
#endif

#ifdef CPPCMS_HAS_DECLSPEC 
#   if defined(CPPCMS_ALL_DYN_LINK) || defined(CPPCMS_LOCALE_DYN_LINK)
#       ifdef CPPCMS_LOCALE_SOURCE
#           define CPPCMS_API __declspec(dllexport)
#       else
#           define CPPCMS_API __declspec(dllimport)
#       endif  // CPPCMS_LOCALE_SOURCE
#   endif  // DYN_LINK
#endif  // CPPCMS_HAS_DECLSPEC

#ifndef CPPCMS_API
#   define CPPCMS_API
#endif


#endif // boost/locale/config.hpp
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

