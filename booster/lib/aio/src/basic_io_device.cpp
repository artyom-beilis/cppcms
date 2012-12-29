//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include "socket_details.h"

#include <booster/aio/basic_io_device.h>
#include <booster/aio/io_service.h>
#include <booster/aio/aio_category.h>

#include "category.h"

//#define BOOSTER_AIO_FORCE_POLL


namespace booster {
namespace aio {

struct basic_io_device::data{}; // for future use

basic_io_device::basic_io_device() :
	fd_(invalid_socket),
	owner_(true),
	nonblocking_was_set_(false),
	srv_(0)
{
}

basic_io_device::basic_io_device(io_service &srv) :
	fd_(invalid_socket),
	owner_(true),
	nonblocking_was_set_(false),
	srv_(&srv)
{
}

basic_io_device::~basic_io_device()
{
	if(!owner_ || fd_ == invalid_socket)
		return;
	close_file_descriptor(fd_);
	fd_=invalid_socket;
}

bool basic_io_device::has_io_service()
{
	return srv_ != 0;
}

io_service &basic_io_device::get_io_service()
{
	if(!has_io_service())
		throw system::system_error(aio_error::no_service_provided,aio_error_cat);
	return *srv_;
}

void basic_io_device::reset_io_service()
{
	if(has_io_service())
		cancel();
	srv_ = 0;
}

void basic_io_device::set_io_service(io_service &srv)
{
	reset_io_service();
	srv_ = &srv;
}

native_type basic_io_device::native()
{
	return fd_;
}

void basic_io_device::attach(native_type fd)
{
	system::error_code e;
	close(e);
	fd_ = fd;
	owner_ = false;
	nonblocking_was_set_ = false;
}

void basic_io_device::assign(native_type fd)
{
	system::error_code e;
	close(e);
	fd_ = fd;
	owner_ = true;
	nonblocking_was_set_ = false;
}


native_type basic_io_device::release()
{
	owner_ = false;
	return fd_;
}

void basic_io_device::close(system::error_code &e)
{
	if(fd_ == invalid_socket)
		return;
	if(has_io_service())
		cancel();
	if(!owner_)
		return;
	if(close_file_descriptor(fd_))
		e=geterror();
	fd_ = invalid_socket;
	nonblocking_was_set_ = false;
}
void basic_io_device::close()
{
	system::error_code e;
	close(e);
	if(e)
		throw system::system_error(e);
}

void basic_io_device::on_readable(event_handler const &h)
{
	get_io_service().set_io_event(fd_,io_service::in,h);
}

void basic_io_device::on_writeable(event_handler const &h)
{
	get_io_service().set_io_event(fd_,io_service::out,h);
}

void basic_io_device::cancel()
{
	get_io_service().cancel_io_events(fd_);
}

void basic_io_device::set_non_blocking(bool nonblocking,system::error_code &e)
{
	#if defined BOOSTER_WIN32
	unsigned long opt =  nonblocking;
	if(::ioctlsocket(fd_,FIONBIO,&opt) < 0)
		e=geterror();
	#elif ! defined(O_NONBLOCK)
	int opt = nonblocking;
	if(::ioctl(fd_,FIONBIO,&opt) < 0)
		e=geterror();
	#else 
	int flags = ::fcntl(fd_,F_GETFL,0);
	if(flags < 0) {
		e=geterror();
		return;
	}
	if(nonblocking)
		flags = flags | O_NONBLOCK;
	else
		flags = flags & ~O_NONBLOCK;

	if(::fcntl(fd_,F_SETFL,flags) < 0)
		e=geterror();
	#endif
	nonblocking_was_set_=nonblocking;
}

void basic_io_device::set_non_blocking(bool nonblocking)
{
	system::error_code e;
	set_non_blocking(nonblocking,e);
	if(e) throw system::system_error(e);
}

basic_io_device &basic_io_device::lowest_layer()
{
	return *this;
}

bool basic_io_device::would_block(system::error_code const &e)
{
	int code = e.value();
	bool block = 0
	#ifdef EAGAIN
	|| code==EAGAIN
	#endif
	#ifdef EINPROGRESS
	|| code == EINPROGRESS
	#endif
	#ifdef EWOULDBLOCK
	|| code == EWOULDBLOCK
	#endif
	#ifdef WSAEAGAIN
	|| code==WSAEAGAIN
	#endif
	#ifdef WSAEINPROGRESS
	|| code == WSAEINPROGRESS
	#endif
	#ifdef WSAEWOULDBLOCK
	|| code == WSAEWOULDBLOCK
	#endif
	;
	return block;
}

bool basic_io_device::dont_block(io_handler const &h)
{
	if(nonblocking_was_set_)
		return true;
	system::error_code e;
	set_non_blocking(true,e);
	if(e) {
		get_io_service().post(h,e,0);
		return false;
	}
	nonblocking_was_set_ = true;
	return true;
}

bool basic_io_device::dont_block(event_handler const &h)
{
	if(nonblocking_was_set_)
		return true;
	system::error_code e;
	set_non_blocking(true,e);
	if(e) {
		get_io_service().post(h,e);
		return false;
	}
	nonblocking_was_set_ = true;
	return true;
}



} // aio
} // booster



