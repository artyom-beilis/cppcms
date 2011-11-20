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

		///
		/// \brief A timer object
		///
		class BOOSTER_API deadline_timer : public noncopyable {
		public:
			///
			/// Create a new timer object
			///
			deadline_timer();

			///
			/// Create a new timer object connected to the \ref io_service \a srv
			///
			deadline_timer(io_service &srv);
			~deadline_timer();

			///
			/// Returns the connected io_service, throws system::system_error if no \ref io_service connected
			///
			io_service &get_io_service();
			///
			/// Sets new io_service. Cancels all pending asynchronous operations on the connected io_service.
			///
			void set_io_service(io_service &srv);
			///
			/// Unsets the io_service. Cancels all pending asynchronous operations on the connected io_service.
			///
			void reset_io_service();

			///
			/// Set an expiration time relativelty to the current point with an offset \a t
			///
			/// If the function is called during the wait operation the behavior is undefined
			///
			void expires_from_now(ptime t);
			///
			/// Get an expiration time relativelty to the current point with an offset \a t
			///
			ptime expires_from_now();

			///
			/// Set an absolute expiration time
			///
			/// If the function is called during the wait operation the behavior is undefined
			///
			void expires_at(ptime t);
			///
			/// Get an absolute expiration time
			///
			ptime expires_at();

			///
			/// Wait synchronously for the timer
			///
			void wait();
			///
			/// Wait asynchronously for the timer.
			///
			/// If io_service is not assigned throws system::system_error, all other errors reported via \a h
			///
			void async_wait(event_handler const &h);
			///
			/// Cancel asynchronous operation. The active handler is immediately scheduled for execution
			/// with booster::aio::aio_error::cancel error code in booster::aio::aio_error_cat category.
			///
			/// It allows to distinguish between operation cancellation and normal completion 
			///
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
