///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_TCP_MESSENGER_H
#define CPPCMS_TCP_MESSENGER_H
#include <booster/aio/socket.h>
#include <booster/noncopyable.h>
#include "tcp_cache_protocol.h"

namespace cppcms {
namespace impl {
class messenger : public booster::noncopyable {
	booster::aio::stream_socket socket_;
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
