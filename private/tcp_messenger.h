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
#ifndef CPPCMS_TCP_MESSENGER_H
#define CPPCMS_TCP_MESSENGER_H
#include <booster/aio/socket.h>
#include <booster/noncopyable.h>
#include "tcp_cache_protocol.h"

namespace cppcms {
namespace impl {
class messenger : public booster::noncopyable {
	booster::aio::socket socket_;
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
