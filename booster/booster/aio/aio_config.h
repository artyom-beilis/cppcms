//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_CONFIG_H
#define BOOSTER_AIO_CONFIG_H

#include <booster/config.h>
#include <booster/build_config.h>

#if defined(BOOSTER_WIN32)
#  define BOOSTER_AIO_NO_PF_UNIX
#endif

#ifndef BOOSTER_AIO_HAVE_PF_INET6
#  define BOOSTER_AIO_NO_PF_INET6
#endif


#endif // AIO_CONFIG_H
