///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "tcp_messenger.h"
#include "tcp_connector.h"
#include <cppcms/cppcms_error.h>

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
		tcp[i].transmit(ht,dt);
	}
}

unsigned tcp_connector::hash(std::string const &key)
{
	if(conns==1) return 0;
	unsigned h=0;
	// crc 
	for(size_t i=0;i<key.size();i++)  {
		unsigned char c=key[i];

		unsigned highorder = h & 0xf8000000u;
		h<<=5;
		h^=highorder >> 27;
		h^=c;
	}
	return h % conns;
}

messenger &tcp_connector::get(std::string const &key)
{
	return tcp[hash(key)];
}

} // impl
} // cppcms

