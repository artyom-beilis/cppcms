///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "session_tcp_storage.h"
#include "tcp_messenger.h"
#include <cppcms/session_sid.h>
#include <stdio.h>
#include <time.h>

namespace cppcms {
namespace sessions {
using namespace cppcms::impl;
class sessions_tcp_connector : public cppcms::impl::tcp_connector {
public:
	sessions_tcp_connector(std::vector<std::string> const &ips,std::vector<int> const &ports) :
		cppcms::impl::tcp_connector(ips,ports)
	{
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
