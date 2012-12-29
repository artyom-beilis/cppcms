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

struct stream_socket::data{};

stream_socket::stream_socket()
{
}

stream_socket::stream_socket(io_service &s) : basic_socket(s)
{
}

stream_socket::~stream_socket()
{
}


void stream_socket::open(family_type d)
{
	basic_socket::open(d,sock_stream);
}

void stream_socket::open(family_type d,system::error_code &e)
{
	basic_socket::open(d,sock_stream,e);
}


void stream_socket::shutdown(how_type how,system::error_code &e)
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

	int res = ::shutdown(native(),method);
	if(res < 0)
		e=geterror();
}

void stream_socket::shutdown(how_type how)
{
	system::error_code e;
	shutdown(how,e);
	if(e)
		throw system::system_error(e);
}




size_t stream_socket::read_some(mutable_buffer const &buffer,system::error_code &e)
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

size_t stream_socket::write_some(const_buffer const &buffer,system::error_code &e)
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

size_t stream_socket::read_some(mutable_buffer const &buffer)
{
	system::error_code e;
	size_t r=read_some(buffer,e);
	if(e)
		throw system::system_error(e);
	return r;
}

size_t stream_socket::write_some(const_buffer const &buffer)
{
	system::error_code e;
	size_t r=write_some(buffer,e);
	if(e)
		throw system::system_error(e);
	return r;
}


int stream_socket::readv(mutable_buffer const &b)
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
		int ret = ::readv(native(),vec,size);
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
	int res = ::WSARecv(native(),vec,size,&recved,&flags,0,0);
	if(res == 0)
		return recved;
	return -1;
#endif
}



int stream_socket::writev(const_buffer const &b)
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
		int ret = ::writev(native(),vec,size);
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
	int res = ::WSASend(native(),vec,size,&send,0,0,0);
	if(res == 0)
		return send;
	return -1;
#endif
}

void stream_socket::connect(endpoint const &ep,system::error_code &e)
{
	endpoint::native_address_type address = ep.raw();
	#ifndef BOOSTER_WIN32
	for(;;) {
		int res = ::connect(native(),address.first,address.second);
		if(res < 0 && errno==EINTR)
			continue;
		if(res < 0) {
			e=geterror();
			return;
		}
		break;
	}
	#else
	if(::connect(native(),address.first,address.second) < 0)
		e=geterror();
	#endif
}

void stream_socket::connect(endpoint const &ep)
{
	system::error_code e;
	connect(ep,e);
	if(e) throw system::system_error(e);
}

size_t stream_socket::read(mutable_buffer const &buffer,system::error_code &e)
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

size_t stream_socket::write(const_buffer const &buffer,system::error_code &e)
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

size_t stream_socket::read(mutable_buffer const &buf)
{
	system::error_code e;
	size_t n=read(buf,e);
	if(e)
		throw system::system_error(e);
	return n;
}

size_t stream_socket::write(const_buffer const &buf)
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
		stream_socket *sock;

		typedef intrusive_ptr<reader_some> pointer;

		reader_some(io_handler const &ih,mutable_buffer const &ibuf,stream_socket *isock) :
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
				if(n==0 && err && basic_io_device::would_block(err))
					sock->on_readable(pointer(this));
				else
					h(err,n);
			}
		}
	};

	struct writer_some : public booster::callable<void(system::error_code const &e)> {
		io_handler h;
		const_buffer buf;
		stream_socket *sock;
		
		typedef intrusive_ptr<writer_some> pointer;
		
		writer_some(io_handler const &ih,const_buffer const &ibuf,stream_socket *isock) :
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
				if(n==0 && err && basic_io_device::would_block(err))
					sock->on_writeable(pointer(this)); 
				else
					h(err,n);
			}
		}
	};

	struct async_connector : public callable<void(system::error_code const &e)> {
		event_handler h;
		stream_socket *sock;

		async_connector(event_handler const &_h,stream_socket *_s) : h(_h),sock(_s) {}

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
		reader_all(stream_socket *s,mutable_buffer const &b,io_handler const &handler) :
			buf(b),
			count(0),
			self(s),
			h(handler)
		{
		}

		void run()
		{
			#ifdef BOOSTER_AIO_FORCE_POLL
			self->on_readable(intrusive_ptr<reader_all>(this));
			#else
			system::error_code e;
			size_t n = self->read_some(buf,e);
			count+=n;
			buf+=n;
			if(buf.empty() || (e && !basic_io_device::would_block(e))) {
				self->get_io_service().post(h,e,count);
			}
			else {
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
				if(buf.empty() || (err && !basic_io_device::would_block(err))) {
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
		stream_socket *self;
		io_handler h;
		bool via_poll;
	};

	struct writer_all : public callable<void(system::error_code const &e)> 
	{
		typedef intrusive_ptr<writer_all> pointer;
		writer_all(stream_socket *s,const_buffer const &b,io_handler const &handler) :
			buf(b),
			count(0),
			self(s),
			h(handler)
		{
		}

		void run()
		{
			#ifdef BOOSTER_AIO_FORCE_POLL

			self->on_writeable(intrusive_ptr<writer_all>(this));

			#else

			system::error_code e;
			size_t n = self->write_some(buf,e);
			count+=n;
			buf+=n;
			if(buf.empty() || (e && !basic_io_device::would_block(e))) {
				self->get_io_service().post(h,e,count);
			}
			else {
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
				if(buf.empty() || (err && !basic_io_device::would_block(err))) {
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
		stream_socket *self;
		io_handler h;
	};


} // anonymous

void stream_socket::async_write_some(const_buffer const &buffer,io_handler const &h)
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
		get_io_service().post(h,e,n);
	}
	#endif
}

void stream_socket::async_read_some(mutable_buffer const &buffer,io_handler const &h)
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
		get_io_service().post(h,e,n);
	}
	#endif
}

void stream_socket::async_connect(endpoint const &ep,event_handler const &h)
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
		get_io_service().post(h,e);
	}
}

void stream_socket::async_read(mutable_buffer const &buffer,io_handler const &h)
{
	if(!dont_block(h))
		return;
	reader_all::pointer r(new reader_all(this,buffer,h));
	r->run();
}


void stream_socket::async_write(const_buffer const &buffer,io_handler const &h)
{
	if(!dont_block(h))
		return;
	writer_all::pointer r(new writer_all(this,buffer,h));
	r->run();
}


void socket_pair(stream_socket &s1,stream_socket &s2,system::error_code &e)
{
	try {
		socket_pair(s1,s2);
	}
	catch(system::system_error const &err) {
		e=err.code();
	}
}

void socket_pair(stream_socket &s1,stream_socket &s2)
{
#ifdef BOOSTER_AIO_NO_PF_UNIX
	acceptor a;
	a.open(pf_inet);
	a.set_option(acceptor::reuse_address,true);
	a.bind(endpoint("127.0.0.1",0));
	a.listen(1);
	s1.open(pf_inet);
	s1.connect(a.local_endpoint());
	a.accept(s2);
	a.close();
#else
	int fds[2];
	if(::socketpair(AF_UNIX,SOCK_STREAM,0,fds) < 0)
		throw system::system_error(geterror());
	s1.assign(fds[0]);
	s2.assign(fds[1]);
#endif
}



} // aio
} // booster



