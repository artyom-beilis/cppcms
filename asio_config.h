#ifndef CPPCMS_ASIO_CONF_H
#define CPPCMS_ASIO_CONF_H

#include "config.h"
#ifdef __CYGWIN__
// Cygwin ASIO works only with win32 sockets
#define _WIN32_WINNT 1
#define __USE_W32_SOCKETS 1
#endif

#ifdef USE_BOOST_ASIO
#include <boost/asio.hpp>
namespace aio = boost::asio;
using boost::system::error_code;
using boost::system::system_error;
#else
#include <asio.hpp>
namespace aio = asio;
using asio::error_code;
using asio::system_error;
#endif

using aio::ip::tcp;


#endif
