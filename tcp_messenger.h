#ifndef CPPCMS_TCP_MESSENGER_H
#define CPPCMS_TCP_MESSENGER_H

#include "asio_config.h"
// MUST BE FIRST

#include "tcp_cache_protocol.h"
#include "archive.h"


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
