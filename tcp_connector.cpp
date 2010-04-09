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
#define CPPCMS_SOURCE
#include "asio_config.h"
// MUST BE FIRST TO COMPILE CORRECTLY UNDER CYGWIN
#include "tcp_messenger.h"
#include "tcp_connector.h"
#include "cppcms_error.h"

namespace cppcms {
namespace impl {

tcp_connector::tcp_connector(std::vector<std::string> const& ip,std::vector<int> const &port)
{
	if(ip.size()<1 || port.size()!=ip.size()) {
		throw cppcms_error("Incorrect parameters for tcp cache");
	}
	conns=ip.size();
	tcp=new messenger[conns];
	try {
		for(int i=0;i<conns;i++) {
			tcp[i].connect(ip[i],port[i]);
		}
	}
	catch(...) {
		delete [] tcp;
		tcp=NULL;
		throw;
	}
}

tcp_connector::~tcp_connector()
{
	delete [] tcp;
}

void tcp_connector::broadcast(tcp_operation_header &h,std::string &data)
{
	int i;
	for(i=0;i<conns;i++) {
		tcp_operation_header ht=h;
		std::string dt=data;
		tcp[i].transmit(ht,data);
	}
}

unsigned tcp_connector::hash(std::string const &key)
{
	if(conns==1) return 0;
	unsigned val=0,i;
	for(i=0;i<key.size();i++) {
		val+=251*key[i]+103 % 307;
	}
	return val % conns;;
}

messenger &tcp_connector::get(std::string const &key)
{
	return tcp[hash(key)];
}

} // impl
} // cppcms

