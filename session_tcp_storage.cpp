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
#include "tcp_messenger.h"
#include "session_tcp_storage.h"
#include "session_sid.h"
#include "global_config.h"

namespace cppcms {

unsigned session_tcp_storage::hash(string const &key)
{
	if(conns==1) return 0;
	char buf[5] = { key.at(0) , key.at(1), key.at(2), key.at(3) , 0};
	unsigned val=0;
	sscanf(buf,"%x",&val);
	return val % conns;;
}

void session_tcp_storage::save(std::string const &sid,time_t timeout,std::string const &in)
{
	time_t now=time(NULL);
	if(now > timeout) {
		return ;
	}
	tcp_operation_header h={0};
	h.opcode=opcodes::session_save;
	h.size=in.size() + 32;
	h.operations.session_save.timeout=timeout - now;
	string data=sid;
	data.append(in.begin(),in.end());
	get(sid).transmit(h,data);
}

bool session_tcp_storage::load(std::string const &sid,time_t *timeout,std::string &out)
{
	tcp_operation_header h={0};
	h.opcode=opcodes::session_load;
	h.size=sid.size();
	string data=sid;
	get(sid).transmit(h,data);
	if(h.opcode==opcodes::session_load_data) {
		if(timeout) *timeout=time(NULL) + h.operations.session_data.timeout;
		out.swap(data);
		return true;
	}
	else {
		return false;
	}
	
}

void session_tcp_storage::remove(std::string const &sid)
{
	tcp_operation_header h={0};
	h.opcode=opcodes::session_remove;
	h.size=sid.size();
	string data=sid;
	get(sid).transmit(h,data);
}

namespace {

struct builder {
	vector<string> ips;
	vector<int> ports;
	builder(vector<string> const &ips_,vector<int> const &ports_) :
		ips(ips_), ports(ports_)
	{
	}
	boost::shared_ptr<session_api> operator()(worker_thread &w)
	{
		boost::shared_ptr<session_server_storage> storage(new session_tcp_storage(ips,ports));
		return boost::shared_ptr<session_api>(new session_sid(storage));
	}
} ;

} // anon


session_backend_factory session_tcp_storage::factory(cppcms_config const &conf)
{
	return builder(conf.slist("session.tcp_ips"),conf.ilist("session.tcp_ports"));
}

} // cppcms
