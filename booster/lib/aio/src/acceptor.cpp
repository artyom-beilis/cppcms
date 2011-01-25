//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
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

struct acceptor::data{};

acceptor::acceptor()
{
}

acceptor::acceptor(io_service &s) : basic_socket(s)
{
}

acceptor::~acceptor()
{
}

void acceptor::open(family_type d)
{
	basic_socket::open(d,sock_stream);
}

void acceptor::open(family_type d,system::error_code &e)
{
	basic_socket::open(d,sock_stream,e);
}

void acceptor::accept(stream_socket &target)
{
	system::error_code e;
	accept(target,e);
	if(e)
		throw system::system_error(e);
}

void acceptor::accept(stream_socket &target,system::error_code &e)
{
	native_type new_fd = invalid_socket;
	#ifndef BOOSTER_WIN32
	for(;;) {
		new_fd = ::accept(native(),0,0);
		if(new_fd < 0 && errno==EINTR)
			continue;
		break;
	}
	#else
	new_fd = ::accept(native(),0,0);
	#endif

	if(new_fd == invalid_socket) {
		e=geterror();
		return;
	}

	target.assign(new_fd);
	return;
}

void acceptor::listen(int backlog,system::error_code &e)
{
	if(::listen(native(),backlog) < 0)
		e=geterror();
}

void acceptor::listen(int backlog)
{
	system::error_code e;
	listen(backlog,e);
	if(e)
		throw system::system_error(e);
}

void acceptor::bind(endpoint const &ep,system::error_code &e)
{
	endpoint::native_address_type address = ep.raw();
	if(::bind(native(),address.first,address.second) < 0)
		e=geterror();
}

void acceptor::bind(endpoint const &ep)
{
	system::error_code e;
	bind(ep,e);
	if(e)
		throw system::system_error(e);
}

namespace {
	struct async_acceptor : public callable<void(system::error_code const &e)> {
		event_handler h;
		stream_socket *target;
		acceptor *source;
		async_acceptor(event_handler const &_h,stream_socket *_t,acceptor *_s) : h(_h),target(_t),source(_s) {}

		typedef std::auto_ptr<async_acceptor> pointer;

		void operator()(system::error_code const &e)
		{
			if(e) { h(e); return; }
			system::error_code reserr;
			source->accept(*target,reserr);
			if(basic_io_device::would_block(reserr)) {
				source->async_accept(*target,h);
			}
			else
				h(reserr);
		}
	};
} // anonymous

void acceptor::async_accept(stream_socket &target,event_handler const &h)
{
	if(!dont_block(h))
		return;
	async_acceptor::pointer acceptor(new async_acceptor( h, &target, this ));
	on_readable(acceptor);
}


} // aio
} // booster



