//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_DEADLINE_TIMER_H
#define BOOSTER_AIO_DEADLINE_TIMER_H

#include <booster/config.h>
#include <booster/posix_time.h>
#include <booster/hold_ptr.h>
#include <booster/callback.h>
#include <booster/aio/types.h>
#include <booster/noncopyable.h>

namespace booster {
	namespace aio {

		class BOOSTER_API deadline_timer : public noncopyable {
		public:
			deadline_timer();
			deadline_timer(io_service &srv);
			~deadline_timer();

			io_service &get_io_service();
			void set_io_service(io_service &srv);
			void reset_io_service();

			void expires_from_now(ptime t);
			ptime expires_from_now();

			void expires_at(ptime t);
			ptime expires_at();

			void wait();
			void async_wait(event_handler const &h);
			void cancel();
		private:
			struct waiter;
			friend struct waiter;

			struct data;
			hold_ptr<data> d;

			io_service *srv_;
			ptime deadline_;
			int event_id_;

		};

	} // aio
} // booster



#endif
