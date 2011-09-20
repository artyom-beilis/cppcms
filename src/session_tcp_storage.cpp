///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2011  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
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
#define CPPCMS_SOURCE
#include "session_tcp_storage.h"
#include "tcp_messenger.h"
#include <cppcms/session_sid.h>
#include <stdio.h>

namespace cppcms {
namespace sessions {
using namespace cppcms::impl;
class sessions_tcp_connector : public cppcms::impl::tcp_connector {
public:
	sessions_tcp_connector(std::vector<std::string> const &ips,std::vector<int> const &ports) :
		cppcms::impl::tcp_connector(ips,ports)
	{
	}
	unsigned hash(std::string const &key)
	{
		if(conns==1) return 0;
		char buf[5] = { key.at(0) , key.at(1), key.at(2), key.at(3) , 0};
		unsigned val=0;
		sscanf(buf,"%x",&val);
		return val % conns;;
	}
};

bool tcp_storage::is_blocking()
{
	return true;
}

void tcp_storage::save(std::string const &sid,time_t timeout,std::string const &in)
{
	tcp_operation_header h=tcp_operation_header();
	h.opcode=opcodes::session_save;
	h.size=in.size() + 32;
	h.operations.session_save.timeout=timeout - time(0);
	std::string data;
	data.reserve(sid.size() + in.size());
	data+=sid;
	data+=in;
	tcp().get(sid).transmit(h,data);
}

bool tcp_storage::load(std::string const &sid,time_t &timeout,std::string &out)
{
	tcp_operation_header h=tcp_operation_header();
	h.opcode=opcodes::session_load;
	h.size=sid.size();
	std::string data=sid;
	tcp().get(sid).transmit(h,data);
	if(h.opcode==opcodes::session_load_data) {
		timeout = time(NULL) + h.operations.session_data.timeout;
		out.swap(data);
		return true;
	}
	else {
		return false;
	}
	
}

void tcp_storage::remove(std::string const &sid)
{
	tcp_operation_header h=tcp_operation_header();
	h.opcode=opcodes::session_remove;
	h.size=sid.size();
	std::string data=sid;
	tcp().get(sid).transmit(h,data);
}
tcp_connector &tcp_storage::tcp()
{
	tcp_connector *p = tcp_.get();
	if(!p) {
		p=new sessions_tcp_connector(ips_,ports_);
		tcp_.reset(p);
	}
	return *p;
}

} // sessions
} // cppcms
