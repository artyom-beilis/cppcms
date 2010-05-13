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
	socket_.open(ep.family(),booster::aio::sock_stream,e);
	if(!e)
		socket_.connect(ep,e);
	if(e) 
		throw cppcms_error("connect:"+e.message());
	socket_.set_option(booster::aio::socket::tcp_no_delay,true);
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
			// FIXME use buffers
			socket_.write(booster::aio::buffer(&h,sizeof(h)));
			if(h.size>0) {
				socket_.write(booster::aio::buffer(data.c_str(),h.size));
			}
			socket_.read(booster::aio::buffer(&h,sizeof(h)));
			if(h.size>0) {
				std::vector<char> d(h.size);
				socket_.read(booster::aio::buffer(d));
				data.assign(d.begin(),d.begin()+h.size);
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
			socket_.open(ep.family(),booster::aio::sock_stream,er);
			if(!er)
				socket_.connect(ep,er);
			if(er) throw cppcms_error("reconnect:"+er.message());
			times++;
		}
	}while(!done);
} 

} // impl
	
} // namespace cppcms

