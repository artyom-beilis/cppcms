#ifndef CPPCMS_TCP_MESSENGER_H
#define CPPCMS_TCP_MESSENGER_H

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

#include "tcp_cache_protocol.h"
#include "archive.h"

using aio::ip::tcp;

namespace cppcms {

class messenger : boost::noncopyable {
	aio::io_service srv_;
	tcp::socket socket_;
	string ip_;
	int port_;
public:
	void connect(string ip,int port);
	messenger(string ip,int port);
	messenger();
	void transmit(tcp_operation_header &h,string &data);	
};

} // cppcms

#endif
