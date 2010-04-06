#ifndef CPPCMS_TCP_MESSENGER_H
#define CPPCMS_TCP_MESSENGER_H
#include "asio_config.h"
// MUST BE FIRST
#include "noncopyable.h"
#include "tcp_cache_protocol.h"

namespace cppcms {
namespace impl {
class messenger : public util::noncopyable {
	boost::asio::io_service srv_;
	boost::asio::ip::tcp::socket socket_;
	std::string ip_;
	int port_;
public:
	void connect(std::string ip,int port);
	messenger(std::string ip,int port);
	messenger();
	void transmit(tcp_operation_header &h,std::string &data);	
};
} // impl
} // cppcms

#endif
