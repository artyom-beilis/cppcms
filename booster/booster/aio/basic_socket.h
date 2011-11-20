//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_BASIC_SOCKET_H
#define BOOSTER_AIO_BASIC_SOCKET_H

#include <booster/aio/types.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/basic_io_device.h>

namespace booster {
namespace aio {
	
	class io_service;
	

	///
	/// \brief This class represents a basic Socket object.
	///	
	class BOOSTER_API basic_socket : public basic_io_device {
	public:
		///
		/// Create a new socket object
		///
		basic_socket();
		///
		/// Create a new socket object and connect to the io_service \a srv
		///
		basic_socket(io_service &srv);
		~basic_socket();

		///
		/// Open a socket of \ref family_type \a d and of the protocol (\ref socket_type) \a t
		///
		/// Throws system::system_error if error occurs.
		///	
		void open(family_type d,socket_type t);
		///
		/// Opens a new stream socket of a \ref family_type \a d
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void open(family_type d,socket_type t,system::error_code &e);

		///
		/// Get a local endpoint for the socket
		///
		/// If a error occurs it is assigned to \a e.
		///	
		endpoint local_endpoint(system::error_code &e);
		///
		/// Get a local endpoint for the socket
		///
		/// Throws system::system_error if error occurs.
		///	
		endpoint local_endpoint();

		///
		/// Get a remote endpoint for the socket
		///
		/// If a error occurs it is assigned to \a e.
		///	
		endpoint remote_endpoint(system::error_code &e);
		///
		/// Get a remote endpoint for the socket
		///
		/// Throws system::system_error if error occurs.
		///	
		endpoint remote_endpoint();

		///
		/// Boolean socket options list
		///
		typedef enum {
			tcp_no_delay,	//< TCP_NODELAY options - disables Nagle's algorithm
			keep_alive,	//< SO_KEEPALIVE socket option 
			reuse_address	//< SO_REUSEADDR socket option
		} boolean_option_type;

		///
		/// Get a value for a boolean_option_type
		///
		/// If a error occurs it is assigned to \a e.
		///	
		bool get_option(boolean_option_type opt,system::error_code &e);
		///
		/// Get a value for a boolean_option_type
		/// Throws system::system_error if error occurs.
		///	
		bool get_option(boolean_option_type opt);
		///
		/// Set a value for a boolean_option_type
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void set_option(boolean_option_type opt,bool v,system::error_code &e);
		///
		/// Set a value for a boolean_option_type
		///
		/// Throws system::system_error if error occurs.
		///	
		void set_option(boolean_option_type opt,bool v);
		
	private:
		struct data;
		hold_ptr<data> d;
	};

} // aio
} // booster

#endif
