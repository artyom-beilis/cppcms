//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
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
	
	
	
	///
	/// \brief This object represents a stream socket: TCP/IP IPv4 or IPv6 or Unix domain
	/// stream socket
	///
	class BOOSTER_API stream_socket : public basic_socket {
	public:
		///
		/// Shutdown option type
		///
		typedef enum {
			shut_rd, //< Shutdown read operations 
			shut_wr, //< Shutdown write operations
			shut_rdwr //< Shutdown both read and write operations
		} how_type;

		///
		/// Create a new (closed) stream socket
		///
		stream_socket();
		///
		/// Create a new (closed) stream socket and attach the object to \ref io_service \a srv
		///
		stream_socket(io_service &srv);
		~stream_socket();

		///
		/// Opens a new stream socket of a \ref family_type \a d
		///
		/// Throws system::system_error if error occurs.
		///	
		void open(family_type d);
		///
		/// Opens a new stream socket of a \ref family_type \a d
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void open(family_type d,system::error_code &e);

		///
		/// Notify the other side on connection shutdown of a type \a h
		///
		/// Throws system::system_error if error occurs.
		///	
		void shutdown(how_type h);
		///
		/// Notify the other side on connection shutdown of a type \a h
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void shutdown(how_type h,system::error_code &e);

		///
		/// Connect to the remote endpoint \a ep
		///
		/// Throws system::system_error if error occurs.
		///	
		void connect(endpoint const &ep);
		///
		/// Connect to the remote endpoint \a ep
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void connect(endpoint const &ep,system::error_code &e);
		///
		/// Connect asynchronously to the remote endpoint \a ep.
		///
		/// If io_service is not assigned throws system::system_error, all other errors reported via 
		/// the callback \a h.
		///
		void async_connect(endpoint const &ep,event_handler const &h);

		///
		/// Read from the socket to the \a buffer. Returns the number of bytes transfered 
		///
		/// Throws system::system_error if error occurs.
		///	
		size_t read_some(mutable_buffer const &buffer);
		///
		/// Read from the socket to the \a buffer. Returns the number of bytes transfered 
		///
		/// If a error occurs it is assigned to \a e.
		///	
		size_t read_some(mutable_buffer const &buffer,system::error_code &e);
		
		///
		/// Write to the socket from the \a buffer. Returns the number of bytes transfered 
		///
		/// Throws system::system_error if error occurs.
		///	
		size_t write_some(const_buffer const &buffer);
		///
		/// Write to the socket from the \a buffer. Returns the number of bytes transfered 
		///
		/// If a error occurs it is assigned to \a e.
		///	
		size_t write_some(const_buffer const &buffer,system::error_code &e);

		///
		/// Read from the socket to the \a buffer until all required data is transferred.
		/// Returns the number of bytes transfered 
		///
		/// Throws system::system_error if error occurs.
		///	
		size_t read(mutable_buffer const &buffer);
		///
		/// Write to the socket from the \a buffer until all required data is transferred.
		/// Returns the number of bytes transfered 
		///
		/// Throws system::system_error if error occurs.
		///	
		size_t write(const_buffer const &buffer);
		
		///
		/// Read from the socket to the \a buffer until all required data is transferred.
		/// Returns the number of bytes transfered 
		///
		/// If a error occurs it is assigned to \a e.
		///	
		size_t read(mutable_buffer const &buffer,system::error_code &e);
		///
		/// Write to the socket from the \a buffer until all required data is transferred.
		/// Returns the number of bytes transfered 
		///
		/// If a error occurs it is assigned to \a e.
		///	
		size_t write(const_buffer const &buffer,system::error_code &e);

		///
		/// Read asynchronously from the socket to the \a buffer.
		///
		/// The error and the amount of bytes that are transfered are reported via callback.
		/// During the operation the buffers (the data range it points to) must remain valid.
		///
		/// If io_service is not assigned throws system::system_error, all other errors reported via 
		/// the callback \a h.
		///
		void async_read_some(mutable_buffer const &buffer,io_handler const &h);
		///
		/// Write asynchronously to the socket from the \a buffer.
		///
		/// The error and the amount of bytes that are transfered are reported via callback.
		/// During the operation the buffers (the data range it points to) must remain valid.
		///
		/// If io_service is not assigned throws system::system_error, all other errors reported via 
		/// the callback \a h.
		///
		void async_write_some(const_buffer const &buffer,io_handler const &h);

		///
		/// Read asynchronously from the socket to the \a buffer until all required mount of data is transfered.
		///
		/// The error and the amount of bytes that are transfered are reported via callback.
		/// During the operation the buffers (the data range it points to) must remain valid.
		///
		/// If io_service is not assigned throws system::system_error, all other errors reported via 
		/// the callback \a h.
		///
		void async_read(mutable_buffer const &buffer,io_handler const &h);
		///
		/// Write asynchronously to the socket from the \a buffer until all required mount of data is transfered.
		///
		/// The error and the amount of bytes that are transfered are reported via callback.
		/// During the operation the buffers (the data range it points to) must remain valid.
		///
		/// If io_service is not assigned throws system::system_error, all other errors reported via 
		/// the callback \a h.
		///
		void async_write(const_buffer const &buffer,io_handler const &h);

		///
		/// Return a number of avalible bytes to read, if error occurs returns 0 and e set to the error code
		///
		size_t bytes_readable(booster::system::error_code &e);

		///
		/// Return a number of avalible bytes to read, if error occurs system_error is thrown
		///
		size_t bytes_readable();

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
