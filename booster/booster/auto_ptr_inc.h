//
//  Copyright (C) 2009-2017 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AUTO_PTR_INC_H
#define BOOSTER_AUTO_PTR_INC_H

#if __cplusplus >= 201103L
#  include <functional>
#  if defined __GLIBCXX__
#   include <bits/c++config.h>
#   ifdef _GLIBCXX_DEPRECATED
#     undef _GLIBCXX_DEPRECATED
#     define _GLIBCXX_DEPRECATED
#   endif
#  endif
#endif

#include <memory>

#endif // BOOSTER_AUTO_PTR_INC_H
