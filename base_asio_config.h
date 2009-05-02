#ifndef CPPCMS_BASE_ASIO_CONFIG_H
#define CPPCMS_BASE_ASIO_CONFIG_H
#include "config.h"

#ifdef USE_BOOST_ASIO
namespace boost { namespace asio {
	class io_service;
} }
namespace aio = ::boost::asio;
#else
namespace asio {
	class io_service;
}
namespace aio = ::asio;
#endif // No ASIO

#endif
