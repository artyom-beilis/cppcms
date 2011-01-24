//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_STREAM_SOCKET_H
#define BOOSTER_AIO_STREAM_SOCKET_H

#include <booster/aio/types.h>
#include <booster/callback.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/basic_io_device.h>
#include <booster/aio/basic_socket.h>

namespace booster {
namespace aio {
	class mutable_buffer;
	class const_buffer;
	class io_service;
	
	
	
	class BOOSTER_API stream_socket : public basic_socket {
	public:
		typedef enum {
			shut_rd,shut_wr,shut_rdwr
		} how_type;

		stream_socket();
		stream_socket(io_service &srv);
		~stream_socket();

		void open(family_type d);
		void open(family_type d,system::error_code &e);

		void shutdown(how_type h);
		void shutdown(how_type h,system::error_code &e);

		void connect(endpoint const &);
		void connect(endpoint const &,system::error_code &e);
		void async_connect(endpoint const &,event_handler const &h);

		size_t read_some(mutable_buffer const &buffer);
		size_t read_some(mutable_buffer const &buffer,system::error_code &e);
		
		size_t write_some(const_buffer const &buffer);
		size_t write_some(const_buffer const &buffer,system::error_code &e);

		size_t read(mutable_buffer const &buffer);
		size_t write(const_buffer const &buffer);
		
		size_t read(mutable_buffer const &buffer,system::error_code &e);
		size_t write(const_buffer const &buffer,system::error_code &e);

		void async_read_some(mutable_buffer const &buffer,io_handler const &h);
		void async_write_some(const_buffer const &buffer,io_handler const &h);

		void async_read(mutable_buffer const &buffer,io_handler const &h);
		void async_write(const_buffer const &buffer,io_handler const &h);

	private:
		int readv(mutable_buffer const &b);
		int writev(const_buffer const &b);

		struct data;
		hold_ptr<data> d;
	};

	///
	/// Create a connected pair of stream sockets, under UNIX creates unix-domain-sockets
	/// under windows AF_INET sockets
	///
	BOOSTER_API void socket_pair(stream_socket &s1,stream_socket &s2,system::error_code &e);
	///
	/// Create a connected pair of stream sockets, under UNIX creates unix-domain-sockets
	/// under windows AF_INET sockets, throws booster::system_error on error
	///
	BOOSTER_API void socket_pair(stream_socket &s1,stream_socket &s2);
	

} // aio
} // booster

#endif
