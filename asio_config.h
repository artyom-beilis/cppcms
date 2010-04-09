///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
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
