//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_CONFIG_H_INCLUDED
#define BOOSTER_LOCALE_CONFIG_H_INCLUDED

#include <booster/config.h>

#ifdef BOOSTER_HAS_DECLSPEC 
#   if defined(BOOSTER_ALL_DYN_LINK) || defined(BOOSTER_LOCALE_DYN_LINK)
#       ifdef BOOSTER_SOURCE
#           define BOOSTER_API __declspec(dllexport)
#       else
#           define BOOSTER_API __declspec(dllimport)
#       endif  // BOOST_LOCALE_SOURCE
#   endif  // DYN_LINK
#endif  // BOOST_HAS_DECLSPEC

#ifndef BOOSTER_API
#   define BOOSTER_API
#endif


#endif // boost/locale/config.hpp
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

