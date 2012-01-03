//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include "socket_details.h"
#include <booster/aio/socket.h>

//#define BOOSTER_AIO_FORCE_POLL


namespace booster {
namespace aio {

struct basic_socket::data{};

basic_socket::basic_socket()
{
}

basic_socket::basic_socket(io_service &srv) : basic_io_device(srv)
{
}

basic_socket::~basic_socket()
{
}

void basic_socket::open(family_type d,socket_type t,system::error_code &e)
{
	int domain=0;
	switch(d) {
	case pf_unix: domain=AF_UNIX; break;
	case pf_inet: domain=AF_INET; break;
	case pf_inet6: domain = AF_INET6; break;
	}
	int type = 0;
	switch(t) {
	case sock_stream: type = SOCK_STREAM; break;
	case sock_datagram: type = SOCK_DGRAM; break;
	}

	{
		system::error_code etmp;
		close(etmp);
	}
	native_type fd = ::socket(domain,type,0);
	if(fd == invalid_socket) {
		e=geterror();
		return;
	}
	assign(fd);
}

void basic_socket::open(family_type d,socket_type t)
{
	system::error_code e;
	open(d,t,e);
	if(e)
		throw system::system_error(e);
}

endpoint basic_socket::local_endpoint(system::error_code &e)
{
	std::vector<char> endpoint_raw_(1000,0);
	sockaddr *sa = reinterpret_cast<sockaddr *>(&endpoint_raw_.front());
	socklen_t len = endpoint_raw_.size();
	if(::getsockname(native(),sa,&len) < 0)
		e=geterror();
	endpoint ep;
	ep.raw(sa,len);
	return ep;
}

endpoint basic_socket::local_endpoint()
{
	system::error_code e;
	endpoint ep=local_endpoint(e);
	if(e) throw system::system_error(e);
	return ep;
}

endpoint basic_socket::remote_endpoint(system::error_code &e)
{
	std::vector<char> endpoint_raw_(1000,0);
	sockaddr *sa = reinterpret_cast<sockaddr *>(&endpoint_raw_.front());
	socklen_t len = endpoint_raw_.size();
	if(::getpeername(native(),sa,&len) < 0)
		e=geterror();
	endpoint ep;
	ep.raw(sa,len);
	return ep;
}

endpoint basic_socket::remote_endpoint()
{
	system::error_code e;
	endpoint ep=remote_endpoint(e);
	if(e) throw system::system_error(e);
	return ep;
}


void basic_socket::set_option(boolean_option_type opt,bool v,system::error_code &e)
{
	int value = v ? 1 : 0;
	char const *p=reinterpret_cast<char const *>(&value);
	int res = 0;
	switch(opt) {
	case tcp_no_delay:
		res=::setsockopt(native(),IPPROTO_TCP,TCP_NODELAY,p,sizeof(value));
		break;
	case keep_alive:
		res=::setsockopt(native(),SOL_SOCKET,SO_KEEPALIVE,p,sizeof(value));
		break;
	case reuse_address:
		res=::setsockopt(native(),SOL_SOCKET,SO_REUSEADDR,p,sizeof(value));
		break;
	default:
		;
	}
	if(res < 0)
		e=geterror();
}

void basic_socket::set_option(boolean_option_type opt,bool v)
{
	system::error_code e;
	set_option(opt,v,e);
	if(e) throw system::system_error(e);
}

bool basic_socket::get_option(boolean_option_type opt,system::error_code &e)
{
	int value = 0;
	socklen_t len = sizeof(value);
	#ifdef BOOSTER_WIN32
	char *ptr = reinterpret_cast<char *>(&value);
	#else
	int *ptr = &value;
	#endif
	int res = 0;
	switch(opt) {
	case tcp_no_delay:
		res=::getsockopt(native(),IPPROTO_TCP,TCP_NODELAY,ptr,&len);
		break;
	case keep_alive:
		res=::getsockopt(native(),SOL_SOCKET,SO_KEEPALIVE,ptr,&len);
		break;
	case reuse_address:
		res=::getsockopt(native(),SOL_SOCKET,SO_REUSEADDR,ptr,&len);
		break;
	default:
		;
	}
	if(res < 0) {
		e=geterror();
		return false;
	}
	return value!=0;
}

bool basic_socket::get_option(boolean_option_type opt)
{
	system::error_code e;
	bool res = get_option(opt,e);
	if(e) throw system::system_error(e);
	return res;
}


} // aio
} // booster



