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
#include <booster/callback.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/basic_io_device.h>

namespace booster {
namespace aio {
	
	class io_service;
	
	
	class BOOSTER_API basic_socket : public basic_io_device {
	public:
		basic_socket();
		basic_socket(io_service &srv);
		~basic_socket();

		void open(family_type d,socket_type t);
		void open(family_type d,socket_type t,system::error_code &e);

		endpoint local_endpoint(system::error_code &e);
		endpoint local_endpoint();

		endpoint remote_endpoint(system::error_code &e);
		endpoint remote_endpoint();

		typedef enum {
			tcp_no_delay,
			keep_alive,
			reuse_address
		} boolean_option_type;

		bool get_option(boolean_option_type opt,system::error_code &e);
		bool get_option(boolean_option_type opt);
		void set_option(boolean_option_type opt,bool v,system::error_code &e);
		void set_option(boolean_option_type opt,bool v);
		
	private:
		struct data;
		hold_ptr<data> d;
	};

} // aio
} // booster

#endif
