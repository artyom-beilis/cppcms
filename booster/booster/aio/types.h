//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_TYPES_H
#define BOOSTER_AIO_TYPES_H

#include <booster/config.h>
#include <stddef.h>

namespace booster {
	template<typename F>
	class callback;

	namespace system {
		class error_code;
	}


	///
	/// \brief This namespace povides and API to asynchronous sockets API, asynchronous timer
	/// and event loop handing
	///
	namespace aio {
		class endpoint;
		class io_service;

		#ifdef BOOSTER_DOXYGEN_DOCS
		///
		/// Native socket type. int on POSIX platforms and unsigned on Windows.
		///
		typedef unspecified native_type; 
		///
		/// Invalid value for a socket native type
		///
		static const native_type invalid_socket = unspecified;

		#elif defined BOOSTER_WIN32
		typedef unsigned native_type;
		static const unsigned invalid_socket = (unsigned)(-1);
		#else
		typedef int native_type;
		static const int invalid_socket = -1;
		#endif

		///
		/// Completion callback - the callback that receives an operation
		/// completion result - error_code.
		///
		typedef callback<void(system::error_code const &)> event_handler;

		///
		/// General job handler - the operation that should be executed
		///
		typedef callback<void()> handler;
		///
		/// IO completion callback - the callback that receives an operation
		/// completion result - error_code and the amount of bytes transferred 
		///
		typedef callback<void(system::error_code const &,size_t)> io_handler;

		///
		/// Socket family type
		///
		typedef enum {
			pf_unix,  //<  Unix domain socket
			pf_inet,  //<  IPv4 socket
			pf_inet6  //<  IPv6 socket
		} family_type;
		
		///
		/// Socket protocol type
		///
		typedef enum {
			sock_stream,	//< Stream socket (like TCP/IP)
			sock_datagram	//< Datagram socket (like UDP)
		} socket_type;
		
		///
		/// \brief the struct that collects multiple event
		/// types for polling.
		///
		struct io_events {
			static const int in   = 1 << 0; //< Event socket readability
			static const int out  = 1 << 1;	//< Event socket writability
			static const int err  = 1 << 2; //< Error on socket or OOB data
		};
	}
} 


#endif
