//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/config.h>
#ifndef BOOSTER_WIN32
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#else
#include <winsock2.h>
#include <windows.h>
typedef int socklen_t;
#endif

#include <booster/aio/socket.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/io_service.h>
#include <booster/aio/aio_category.h>
#include <booster/aio/buffer.h>


#include <iostream>

#include "category.h"

//#define BOOSTER_AIO_FORCE_POLL


namespace booster {
namespace aio {

struct socket::data{}; // for future use

namespace {
	system::error_code geterror()
	{
		#ifdef BOOSTER_WIN32
		return system::error_code(::WSAGetLastError(),syscat);
		#else
		return system::error_code(errno,syscat);
		#endif
	}
	int close_file_descriptor(int fd)
	{
		#ifndef BOOSTER_WIN32
		int res = 0;
		for(;;) {
			res = ::close(fd);
			if(res < 0 && errno==EINTR)
				continue;
			break;
		}
		#else
		int res = ::closesocket(fd);
		#endif
		return res;
	}



} // anon


socket::socket() :
	fd_(invalid_socket),
	owner_(true),
	nonblocking_was_set_(false),
	srv_(0)
{
}

socket::socket(io_service &srv) :
	fd_(invalid_socket),
	owner_(true),
	nonblocking_was_set_(false),
	srv_(&srv)
{
}

socket::~socket()
{
	if(!owner_ || fd_ == invalid_socket)
		return;
	close_file_descriptor(fd_);
	fd_=invalid_socket;
}

bool socket::has_io_service()
{
	return srv_ != 0;
}

io_service &socket::get_io_service()
{
	if(!has_io_service())
		throw system::system_error(system::error_code(aio_error::no_service_provided,aio_error_cat));
	return *srv_;
}

void socket::reset_io_service()
{
	if(has_io_service())
		cancel();
	srv_ = 0;
}

void socket::set_io_service(io_service &srv)
{
	reset_io_service();
	srv_ = &srv;
}

void socket::open(family_type d,socket_type t,system::error_code &e)
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
	owner_ = true;
	nonblocking_was_set_ = false;
	fd_ = ::socket(domain,type,0);
	if(fd_ == invalid_socket) {
		e=geterror();
		return;
	}
}

void socket::open(family_type d,socket_type t)
{
	system::error_code e;
	open(d,t,e);
	if(e)
		throw system::system_error(e);
}

void socket::shutdown(how_type how,system::error_code &e)
{
	#ifndef SHUT_RDWR
	#define SHUT_RDWR SD_BOTH
	#endif
	#ifndef SHUT_WR
	#define SHUT_WR SD_SEND
	#endif
	#ifndef SHUT_RD
	#define SHUT_RD SD_RECEIVE
	#endif
	int method = 0;
	switch(how) {
	case shut_rd: method = SHUT_RD; break;
	case shut_wr: method = SHUT_WR; break;
	case shut_rdwr: method = SHUT_RDWR; break;
	}

	int res = ::shutdown(fd_,method);
	if(res < 0)
		e=geterror();
}

void socket::shutdown(how_type how)
{
	system::error_code e;
	shutdown(how,e);
	if(e)
		throw system::system_error(e);
}

native_type socket::native()
{
	return fd_;
}

void socket::attach(native_type fd)
{
	system::error_code e;
	close(e);
	fd_ = fd;
	owner_ = false;
	nonblocking_was_set_ = false;
}

void socket::assign(native_type fd)
{
	system::error_code e;
	close(e);
	fd_ = fd;
	owner_ = true;
	nonblocking_was_set_ = false;
}


native_type socket::release()
{
	owner_ = false;
	return fd_;
}

void socket::close(system::error_code &e)
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
void socket::close()
{
	system::error_code e;
	close(e);
	if(e)
		throw system::system_error(e);
}


void socket::accept(socket &target)
{
	system::error_code e;
	accept(target,e);
	if(e)
		throw system::system_error(e);
}

void socket::accept(socket &target,system::error_code &e)
{
	native_type new_fd = invalid_socket;
	#ifndef BOOSTER_WIN32
	for(;;) {
		new_fd = ::accept(fd_,0,0);
		if(new_fd < 0 && errno==EINTR)
			continue;
		break;
	}
	#else
	new_fd = ::accept(fd_,0,0);
	#endif

	if(new_fd == invalid_socket) {
		e=geterror();
		return;
	}

	target.assign(new_fd);
	return;
}

size_t socket::read_some(mutable_buffer const &buffer,system::error_code &e)
{
	int n=readv(buffer);
	if(n < 0) {
		e=geterror();
		return 0;
	}
	if(n == 0) {
		e=system::error_code(aio_error::eof,aio_error_cat);
		return 0;
	}
	return n;
}

size_t socket::write_some(const_buffer const &buffer,system::error_code &e)
{
	int n=writev(buffer);
	if(n < 0) {
		e=geterror();
		return 0;
	}
	if(n == 0) {
		e=system::error_code(aio_error::eof,aio_error_cat);
		return 0;
	}
	return n;
}

size_t socket::read_some(mutable_buffer const &buffer)
{
	system::error_code e;
	size_t r=read_some(buffer,e);
	if(e)
		throw system::system_error(e);
	return r;
}

size_t socket::write_some(const_buffer const &buffer)
{
	system::error_code e;
	size_t r=write_some(buffer,e);
	if(e)
		throw system::system_error(e);
	return r;
}

void socket::on_readable(event_handler const &h)
{
	get_io_service().set_io_event(fd_,io_service::in,h);
}

void socket::on_writeable(event_handler const &h)
{
	get_io_service().set_io_event(fd_,io_service::out,h);
}

void socket::cancel()
{
	get_io_service().cancel_io_events(fd_);
}



int socket::readv(mutable_buffer const &b)
{
	static const unsigned max_vec_size = 16;
	mutable_buffer::buffer_data_type data = b.get();
	unsigned size=0;
#ifndef BOOSTER_WIN32
	struct iovec vec[max_vec_size];
	for(;size < max_vec_size && size < data.second;size++) {
		vec[size].iov_base = data.first[size].ptr;
		vec[size].iov_len = data.first[size].size;
	}
	for(;;) {
		int ret = ::readv(fd_,vec,size);
		if(ret >= 0)
			return ret;
		if(ret < 0 && errno==EINTR)
			continue;
		return ret;
	}
#else // Win32
	WSABUF vec[max_vec_size];
	for(;size < max_vec_size && size < data.second;size++) {
		vec[size].buf = data.first[size].ptr;
		vec[size].len = data.first[size].size;
	}
	DWORD recved=0;
	DWORD flags=0;
	int res = ::WSARecv(fd_,vec,size,&recved,&flags,0,0);
	if(res == 0)
		return recved;
	return -1;
#endif
}



int socket::writev(const_buffer const &b)
{
	static const unsigned max_vec_size = 16;
	const_buffer::buffer_data_type data = b.get();
	unsigned size=0;
#ifndef BOOSTER_WIN32
	struct iovec vec[max_vec_size];
	for(;size < max_vec_size && size < data.second;size++) {
		vec[size].iov_base = const_cast<char *>(data.first[size].ptr);
		vec[size].iov_len = data.first[size].size;
	}
	for(;;) {
		int ret = ::writev(fd_,vec,size);
		if(ret >= 0)
			return ret;
		if(ret < 0 && errno==EINTR)
			continue;
		return ret;
	}
#else // Win32
	WSABUF vec[max_vec_size];
	for(;size < max_vec_size && size < data.second;size++) {
		vec[size].buf = const_cast<char *>(data.first[size].ptr);
		vec[size].len = data.first[size].size;
	}
	DWORD send=0;
	int res = ::WSASend(fd_,vec,size,&send,0,0,0);
	if(res == 0)
		return send;
	return -1;
#endif
}


void socket::listen(int backlog,system::error_code &e)
{
	if(::listen(fd_,backlog) < 0)
		e=geterror();
}

void socket::listen(int backlog)
{
	system::error_code e;
	listen(backlog,e);
	if(e)
		throw system::system_error(e);
}

void socket::bind(endpoint const &ep,system::error_code &e)
{
	endpoint::native_address_type address = ep.raw();
	if(::bind(fd_,address.first,address.second) < 0)
		e=geterror();
}

void socket::bind(endpoint const &ep)
{
	system::error_code e;
	bind(ep,e);
	if(e)
		throw system::system_error(e);
}

void socket::connect(endpoint const &ep,system::error_code &e)
{
	endpoint::native_address_type address = ep.raw();
	#ifndef BOOSTER_WIN32
	for(;;) {
		int res = ::connect(fd_,address.first,address.second);
		if(res < 0 && errno==EINTR)
			continue;
		if(res < 0) {
			e=geterror();
			return;
		}
		break;
	}
	#else
	if(::connect(fd_,address.first,address.second) < 0)
		e=geterror();
	#endif
}

void socket::connect(endpoint const &ep)
{
	system::error_code e;
	connect(ep,e);
	if(e) throw system::system_error(e);
}

void socket::set_option(boolean_option_type opt,bool v,system::error_code &e)
{
	int value = v ? 1 : 0;
	char const *p=reinterpret_cast<char const *>(&value);
	int res = 0;
	switch(opt) {
	case tcp_no_delay:
		res=::setsockopt(fd_,IPPROTO_TCP,TCP_NODELAY,p,sizeof(value));
		break;
	case keep_alive:
		res=::setsockopt(fd_,SOL_SOCKET,SO_KEEPALIVE,p,sizeof(value));
		break;
	case reuse_address:
		res=::setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,p,sizeof(value));
		break;
	default:
		;
	}
	if(res < 0)
		e=geterror();
}

void socket::set_option(boolean_option_type opt,bool v)
{
	system::error_code e;
	set_option(opt,v,e);
	if(e) throw system::system_error(e);
}

bool socket::get_option(boolean_option_type opt,system::error_code &e)
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
		res=::getsockopt(fd_,IPPROTO_TCP,TCP_NODELAY,ptr,&len);
		break;
	case keep_alive:
		res=::getsockopt(fd_,SOL_SOCKET,SO_KEEPALIVE,ptr,&len);
		break;
	case reuse_address:
		res=::getsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,ptr,&len);
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

bool socket::get_option(boolean_option_type opt)
{
	system::error_code e;
	bool res = get_option(opt,e);
	if(e) throw system::system_error(e);
	return res;
}

void socket::set_non_blocking(bool nonblocking,system::error_code &e)
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

void socket::set_non_blocking(bool nonblocking)
{
	system::error_code e;
	set_non_blocking(nonblocking,e);
	if(e) throw system::system_error(e);
}

endpoint socket::local_endpoint(system::error_code &e)
{
	std::vector<char> endpoint_raw_(1000,0);
	sockaddr *sa = reinterpret_cast<sockaddr *>(&endpoint_raw_.front());
	socklen_t len = endpoint_raw_.size();
	if(::getsockname(fd_,sa,&len) < 0)
		e=geterror();
	endpoint ep;
	ep.raw(sa,len);
	return ep;
}

endpoint socket::local_endpoint()
{
	system::error_code e;
	endpoint ep=local_endpoint(e);
	if(e) throw system::system_error(e);
	return ep;
}

endpoint socket::remote_endpoint(system::error_code &e)
{
	std::vector<char> endpoint_raw_(1000,0);
	sockaddr *sa = reinterpret_cast<sockaddr *>(&endpoint_raw_.front());
	socklen_t len = endpoint_raw_.size();
	if(::getpeername(fd_,sa,&len) < 0)
		e=geterror();
	endpoint ep;
	ep.raw(sa,len);
	return ep;
}

endpoint socket::remote_endpoint()
{
	system::error_code e;
	endpoint ep=remote_endpoint(e);
	if(e) throw system::system_error(e);
	return ep;
}

size_t socket::read(mutable_buffer const &buffer,system::error_code &e)
{
	mutable_buffer tmp = buffer;
	size_t count = 0;
	while(!tmp.empty()) {
		size_t n = read_some(tmp,e);
		count+=n;
		if(e) return count;
		tmp+=n;
	}
	return count;
}

size_t socket::write(const_buffer const &buffer,system::error_code &e)
{
	const_buffer tmp = buffer;
	size_t count = 0;
	while(!tmp.empty()) {
		size_t n = write_some(tmp,e);
		count+=n;
		if(e) return count;
		tmp+=n;
	}
	return count;
}

size_t socket::read(mutable_buffer const &buf)
{
	system::error_code e;
	size_t n=read(buf,e);
	if(e)
		throw system::system_error(e);
	return n;
}

size_t socket::write(const_buffer const &buf)
{
	system::error_code e;
	size_t n=write(buf,e);
	if(e)
		throw system::system_error(e);
	return n;
}

namespace {
	struct reader_some : public booster::callable<void(system::error_code const &e)> {
		io_handler h;
		mutable_buffer buf;
		socket *sock;

		typedef intrusive_ptr<reader_some> pointer;

		reader_some(io_handler const &ih,mutable_buffer const &ibuf,socket *isock) :
			h(ih),buf(ibuf),sock(isock)
		{
		}

		void operator()(system::error_code const &e) 
		{
			if(e) {
				h(e,0);
			}
			else {
				system::error_code err;
				size_t n=sock->read_some(buf,err);
				if(n==0 && err && socket::would_block(err))
					sock->on_readable(pointer(this));
				else
					h(err,n);
			}
		}
	};

	struct writer_some : public booster::callable<void(system::error_code const &e)> {
		io_handler h;
		const_buffer buf;
		socket *sock;
		
		typedef intrusive_ptr<writer_some> pointer;
		
		writer_some(io_handler const &ih,const_buffer const &ibuf,socket *isock) :
			h(ih),buf(ibuf),sock(isock)
		{
		}

		void operator()(system::error_code const &e) 
		{
			if(e) {
				h(e,0);
			}
			else {
				system::error_code err;
				size_t n=sock->write_some(buf,err);
				if(n==0 && err && socket::would_block(err))
					sock->on_writeable(pointer(this)); 
				else
					h(err,n);
			}
		}
	};

	struct io_binder : public callable<void()> {
		typedef std::auto_ptr<io_binder> pointer;
		io_handler h;
		size_t n;
		system::error_code e;
		io_binder(io_handler const &ih,size_t in,system::error_code const &ie) : h(ih),n(in),e(ie) {}
		void operator()()
		{
			h(e,n);
		}
	};

	struct event_binder : public callable<void()> {
		event_handler h;
		system::error_code e;
		event_binder(event_handler const &ih,system::error_code const &ie) : h(ih),e(ie) {}
		typedef std::auto_ptr<event_binder> pointer;
		void operator()()
		{
			h(e);
		}
	};

	struct async_acceptor : public callable<void(system::error_code const &e)> {
		event_handler h;
		socket *target;
		socket *source;
		async_acceptor(event_handler const &_h,socket *_t,socket *_s) : h(_h),target(_t),source(_s) {}

		typedef std::auto_ptr<async_acceptor> pointer;

		void operator()(system::error_code const &e)
		{
			if(e) { h(e); return; }
			system::error_code reserr;
			source->accept(*target,reserr);
			if(socket::would_block(reserr)) {
				source->async_accept(*target,h);
			}
			else
				h(reserr);
		}
	};
	struct async_connector : public callable<void(system::error_code const &e)> {
		event_handler h;
		socket *sock;

		async_connector(event_handler const &_h,socket *_s) : h(_h),sock(_s) {}

		typedef std::auto_ptr<async_connector> pointer;

		void operator()(system::error_code const &e)
		{
			if(e) { h(e); return; }
			system::error_code err;
			int errval=0;
			socklen_t len = sizeof(errval);
			#ifdef BOOSTER_WIN32
			char *errptr = reinterpret_cast<char *>(&errval);
			#else
			int *errptr = &errval;
			#endif
			int res = ::getsockopt(sock->native(),SOL_SOCKET,SO_ERROR,errptr,&len);
			if(res < 0) { 
				err=geterror();
			}
			else if(errval!=0) {
				err=system::error_code(errval,syscat);
			}
			h(e);
		}
	};

	struct reader_all : public callable<void(system::error_code const &e)>
	{
		typedef intrusive_ptr<reader_all> pointer;
		reader_all(socket *s,mutable_buffer const &b,io_handler const &handler) :
			count(0),
			self(s)
		{
			#ifdef BOOSTER_AIO_FORCE_POLL
			buf = b;
			h=handler;
			self->on_readable(intrusive_ptr<reader_all>(this));
			#else
			system::error_code e;
			size_t n = s->read_some(b,e);
			count+=n;
			buf=b+n;
			if(buf.empty() || (e && !socket::would_block(e))) {
				io_binder::pointer binder(new io_binder( handler, count, e));
				self->get_io_service().post(binder);
			}
			else {
				h=handler;
				self->on_readable(intrusive_ptr<reader_all>(this));
			}
			#endif
		}
		
		void operator()(system::error_code const &e)
		{
			if(e) {
				h(e,count);
			}
			else {
				system::error_code err;
				size_t n=self->read_some(buf,err);
				count+=n;
				buf+=n;
				if(buf.empty() || (err && !socket::would_block(err))) {
					h(err,count);
				}
				else {
					self->on_readable(intrusive_ptr<reader_all>(this));
				}
			}
		}
	private:
		mutable_buffer buf;
		size_t count;
		socket *self;
		io_handler h;
	};

	struct writer_all : public callable<void(system::error_code const &e)> 
	{
		typedef intrusive_ptr<writer_all> pointer;
		writer_all(socket *s,const_buffer const &b,io_handler const &handler) :
			count(0),
			self(s)
		{
			#ifdef BOOSTER_AIO_FORCE_POLL

			h=handler;
			buf = b;
			self->on_writeable(intrusive_ptr<writer_all>(this));

			#else

			system::error_code e;
			size_t n = s->write_some(b,e);
			count+=n;
			buf=b+n;
			if(buf.empty() || (e && !socket::would_block(e))) {
				io_binder::pointer binder(new io_binder( handler, count, e ));
				self->get_io_service().post(binder);
			}
			else {
				h=handler;
				self->on_writeable(intrusive_ptr<writer_all>(this));
			}

			#endif
		}
		
		void operator()(system::error_code const &e)
		{
			if(e) {
				h(e,count);
			}
			else {
				system::error_code err;
				size_t n=self->write_some(buf,err);
				count+=n;
				buf+=n;
				if(buf.empty() || (err && !socket::would_block(err))) {
					h(err,count);
				}
				else {
					self->on_writeable(intrusive_ptr<writer_all>(this));
				}
			}
		}
	private:
		const_buffer buf;
		size_t count;
		socket *self;
		io_handler h;
	};


} // anonymous

bool socket::would_block(system::error_code const &e)
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

void socket::async_write_some(const_buffer const &buffer,io_handler const &h)
{
	if(!dont_block(h))
		return;
	#ifdef BOOSTER_AIO_FORCE_POLL
	writer_some::pointer writer(new writer_some(h,buffer,this));
	on_writeable(writer);
	#else
	system::error_code e;
	size_t n = write_some(buffer,e);
	if(e && would_block(e)) {
		writer_some::pointer writer(new writer_some(h,buffer,this));
		on_writeable(writer);
	}
	else {
		io_binder::pointer binder(new io_binder( h,n,e ));
		get_io_service().post(binder);
	}
	#endif
}

void socket::async_read_some(mutable_buffer const &buffer,io_handler const &h)
{
	if(!dont_block(h))
		return;
	#ifdef BOOSTER_AIO_FORCE_POLL
	reader_some::pointer reader(new reader_some(h,buffer,this));
	on_readable(reader);
	#else
	system::error_code e;
	size_t n = read_some(buffer,e);
	if(e &&  would_block(e)) {
		reader_some::pointer reader(new reader_some(h,buffer,this));
		on_readable(reader);
	}
	else {
		io_binder::pointer binder(new io_binder( h,n,e ));
		get_io_service().post(binder);
	}
	#endif
}

void socket::async_accept(socket &target,event_handler const &h)
{
	if(!dont_block(h))
		return;
	async_acceptor::pointer acceptor(new async_acceptor( h, &target, this ));
	on_readable(acceptor);
}

void socket::async_connect(endpoint const &ep,event_handler const &h)
{
	if(!dont_block(h))
		return;
	system::error_code e;
	connect(ep,e);
	if(e && would_block(e)) {
		async_connector::pointer connector(new async_connector(h,this));
		on_writeable(connector);
	}
	else {
		event_binder::pointer binder(new event_binder( h,e ));
		get_io_service().post(binder);
	}
}

void socket::async_read(mutable_buffer const &buffer,io_handler const &h)
{
	if(!dont_block(h))
		return;
	reader_all::pointer r(new reader_all(this,buffer,h));
}


void socket::async_write(const_buffer const &buffer,io_handler const &h)
{
	if(!dont_block(h))
		return;
	writer_all::pointer r(new writer_all(this,buffer,h));
}

bool socket::dont_block(io_handler const &h)
{
	if(nonblocking_was_set_)
		return true;
	system::error_code e;
	set_non_blocking(true,e);
	if(e) {
		io_binder::pointer b(new io_binder( h, 0, e));
		get_io_service().post(b);
		return false;
	}
	nonblocking_was_set_ = true;
	return true;
}

bool socket::dont_block(event_handler const &h)
{
	if(nonblocking_was_set_)
		return true;
	system::error_code e;
	set_non_blocking(true,e);
	if(e) {
		event_binder::pointer b(new event_binder( h, e));
		get_io_service().post(b);
		return false;
	}
	nonblocking_was_set_ = true;
	return true;
}


void socket_pair(socket_type t,socket &s1,socket &s2,system::error_code &e)
{
	try {
		socket_pair(t,s1,s2);
	}
	catch(system::system_error const &err) {
		e=err.code();
	}
}
void socket_pair(socket_type t,socket &s1,socket &s2)
{
#ifdef BOOSTER_AIO_NO_PF_UNIX
	socket a;
	a.open(pf_inet,t);
	a.set_option(socket::reuse_address,true);
	a.bind(endpoint("127.0.0.1",0));
	a.listen(1);
	s1.open(pf_inet,sock_stream);
	s1.connect(a.local_endpoint());
	a.accept(s2);
	a.close();
#else
	int type = 0;
	switch(t) {
	case sock_stream: type = SOCK_STREAM; break;
	case sock_datagram: type = SOCK_DGRAM; break;
	}
	int fds[2];
	if(::socketpair(AF_UNIX,type,0,fds) < 0)
		throw system::system_error(geterror());
	s1.assign(fds[0]);
	s2.assign(fds[1]);
#endif
}



} // aio
} // booster



