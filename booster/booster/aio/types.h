//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_TYPES_H
#define BOOSTER_AIO_TYPES_H

#include <booster/config.h>
#include <string.h>

namespace booster {
	template<typename F>
	class callback;

	namespace system {
		class error_code;
	}


	///
	/// \brief This namespace povides and API to asynchronous sockets API
	///
	namespace aio {
		class endpoint;
		class socket;
		class io_service;

		#ifdef BOOSTER_WIN32
		typedef unsigned native_type;
		static const unsigned invalid_socket = (unsigned)(-1);
		#else
		typedef int native_type;
		static const int invalid_socket = -1;
		#endif

		typedef callback<void(system::error_code const &)> event_handler;
		typedef callback<void()> handler;
		typedef callback<void(system::error_code const &,size_t)> io_handler;

		typedef enum {
			pf_unix,
			pf_inet,
			pf_inet6
		} family_type;
		
		typedef enum {
			sock_stream,
			sock_datagram
		} socket_type;
	}
} 


#endif
