//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_REACTOR_CONFIG_H
#define BOOSTER_AIO_REACTOR_CONFIG_H

#if defined __linux

#  define AIO_HAVE_EPOLL
#  define AIO_HAVE_POLL
#  define AIO_HAVE_POSIX_SELECT

#elif defined(__sun) || defined(__hpux)

#  define AIO_HAVE_DEVPOLL
#  define AIO_HAVE_POLL
#  define AIO_HAVE_POSIX_SELECT

#elif defined(__FreeBSD__)

#  define AIO_HAVE_KQUEUE
#  define AIO_HAVE_POLL
#  define AIO_HAVE_POSIX_SELECT

#elif defined(__APPLE__)

// poll(2) is buggy and Mac OS X, removed

#  define AIO_HAVE_KQUEUE
#  define AIO_HAVE_POSIX_SELECT

#elif defined(__CYGWIN__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)

#  define AIO_HAVE_WIN32_SELECT

#else // generic POSIX

#  include <unistd.h>
#  if defined(_POSIX_VERSION) && (_POSIX_VERSION >= 200100)
#    define AIO_HAVE_POLL
#  endif
#  define AIO_HAVE_POSIX_SELECT

#endif


#endif // AIO_CONFIG_H
