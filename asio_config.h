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

#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/asio/read.hpp>
#   include <boost/asio/write.hpp>
#   include <boost/asio/placeholders.hpp>
#   include <boost/asio/deadline_timer.hpp>
#   include <boost/asio/buffer.hpp>
#   include <boost/asio/io_service.hpp>
#   include <boost/asio/ip/tcp.hpp>
#   include <boost/asio/ip/address.hpp>
#   ifndef CPPCMS_WIN32
#      include <boost/asio/local/basic_endpoint.hpp>
#      include <boost/asio/local/connect_pair.hpp>
#      include <boost/asio/local/datagram_protocol.hpp>
#      include <boost/asio/local/stream_protocol.hpp>
#   endif
#else // Internal Boost
#   include <cppcms_boost/asio/read.hpp>
#   include <cppcms_boost/asio/write.hpp>
#   include <cppcms_boost/asio/placeholders.hpp>
#   include <cppcms_boost/asio/deadline_timer.hpp>
#   include <cppcms_boost/asio/buffer.hpp>
#   include <cppcms_boost/asio/io_service.hpp>
#   include <cppcms_boost/asio/ip/tcp.hpp>
#   include <cppcms_boost/asio/ip/address.hpp>
#   ifndef CPPCMS_WIN32
#      include <cppcms_boost/asio/local/basic_endpoint.hpp>
#      include <cppcms_boost/asio/local/connect_pair.hpp>
#      include <cppcms_boost/asio/local/datagram_protocol.hpp>
#      include <cppcms_boost/asio/local/stream_protocol.hpp>
#   endif
    namespace boost = cppcms_boost;
#endif


#endif
