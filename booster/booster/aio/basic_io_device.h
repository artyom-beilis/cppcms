//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_BASIC_IO_DEVICE_H
#define BOOSTER_AIO_BASIC_IO_DEVICE_H

#include <booster/aio/types.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>

namespace booster {
namespace aio {
	class io_service;
	class endpoint;

	class BOOSTER_API basic_io_device : public noncopyable {
	public:
		basic_io_device();
		basic_io_device(io_service &srv);
		~basic_io_device();

		bool has_io_service();
		io_service &get_io_service();
		void set_io_service(io_service &srv);
		void reset_io_service();

		void attach(native_type fd);
		void assign(native_type fd);
		native_type release();
		native_type native();
		
		void close();
		void close(system::error_code &e);

		void on_readable(event_handler const &r);
		void on_writeable(event_handler const &r);
		void cancel();

		basic_io_device &lowest_layer();

		void set_non_blocking(bool nonblocking);
		void set_non_blocking(bool nonblocking,system::error_code &e);

		static bool would_block(system::error_code const &e);

	protected:
		bool dont_block(event_handler const &);
		bool dont_block(io_handler const &);
	private:
		
		struct data;
		hold_ptr<data> d;
		native_type fd_;
		bool owner_;
		bool nonblocking_was_set_;
		io_service *srv_;
	};



} // aio
} // booster

#endif
