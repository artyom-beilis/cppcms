///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "tcp_messenger.h"
#include <cppcms/cppcms_error.h>

#include <booster/aio/socket.h>
#include <booster/aio/buffer.h>
#include <booster/aio/endpoint.h>
#include <booster/system_error.h>

namespace cppcms {
namespace impl {

void messenger::connect(std::string ip,int port) 
{
	ip_=ip;
	port_=port;
	booster::system::error_code e;
	
	booster::aio::endpoint ep(ip,port);
	socket_.open(ep.family(),e);
	if(!e)
		socket_.connect(ep,e);
	if(e) 
		throw cppcms_error("connect:"+e.message());
	socket_.set_option(booster::aio::stream_socket::tcp_no_delay,true);
}

messenger::messenger(std::string ip,int port) :
		socket_()
{
	connect(ip,port);
}
messenger::messenger() :
	socket_()
{
}

void messenger::transmit(tcp_operation_header &h,std::string &data)
{
	bool done=false;
	int times=0;
	do {
		try {
			booster::aio::const_buffer packet = booster::aio::buffer(&h,sizeof(h));
			if(h.size > 0)
				packet += booster::aio::buffer(data.c_str(),h.size);
				
			socket_.write(packet);
			socket_.read(booster::aio::buffer(&h,sizeof(h)));
			if(h.size>0) {
				std::vector<char> d(h.size);
				socket_.read(booster::aio::buffer(d));
				data.assign(d.begin(),d.begin()+h.size);
			}
			else {
				data.clear();
			}
			done=true;
		}
		catch(booster::system::system_error const &e) {
			if(times) {
				throw cppcms_error(std::string("tcp_cache:")+e.what());
			}
			socket_.close();
			booster::aio::endpoint ep(ip_,port_);
			booster::system::error_code er;
			socket_.open(ep.family(),er);
			if(!er)
				socket_.connect(ep,er);
			if(er) throw cppcms_error("reconnect:"+er.message());
			times++;
		}
	}while(!done);
} 

} // impl
	
} // namespace cppcms

