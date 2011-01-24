//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_ACCEPTOR_H
#define BOOSTER_AIO_ACCEPTOR_H

#include <booster/aio/types.h>
#include <booster/callback.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/basic_socket.h>

namespace booster {
namespace aio {
	class io_service;
	class stream_socket;
	
	class BOOSTER_API acceptor : public basic_socket {
	public:

		acceptor();
		acceptor(io_service &srv);
		~acceptor();

		void open(family_type d);
		void open(family_type d,system::error_code &e);

		void accept(stream_socket &);
		void accept(stream_socket &,system::error_code &e);

		void bind(endpoint const &);
		void bind(endpoint const &,system::error_code &e);

		void listen(int backlog);
		void listen(int backlog,system::error_code &e);

		void async_accept(stream_socket &,event_handler const &h);
	
	private:
		struct data;
		hold_ptr<data> d;
	};
	
} // aio
} // booster

#endif
