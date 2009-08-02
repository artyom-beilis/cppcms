#ifndef CPPCMS_ASIO_CONF_H
#define CPPCMS_ASIO_CONF_H

#if defined(_WIN32)
#  define _WIN32_WINNT 0x0500
#elif defined(__CYGWIN__)
#  define _WIN32_WINNT 0x0500
#  define __USE_W32_SOCKETS 1
#endif

#include <boost/asio.hpp>


#endif
