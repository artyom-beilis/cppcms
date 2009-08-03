#ifndef CPPCMS_ASIO_CONF_H
#define CPPCMS_ASIO_CONF_H

#include "defs.h"
#if defined(CPPCMS_WIN32)
#  ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0501
#  endif
#endif

#if defined(CPPCMS_CYGWIN)
#  define __USE_W32_SOCKETS 1
#endif

#include <boost/asio.hpp>


#endif
