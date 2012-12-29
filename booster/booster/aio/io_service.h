//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_IO_SERVICE_H
#define BOOSTER_AIO_IO_SERVICE_H

#include <booster/config.h>
#include <booster/aio/aio_config.h>
#include <booster/aio/types.h>
#include <booster/thread.h>
#include <booster/system_error.h>
#include <booster/aio/aio_category.h>
#include <booster/callback.h>
#include <booster/noncopyable.h>
#include <string>
#include <memory>

namespace booster {
class ptime;
namespace aio {

	class event_loop_impl;


	///
	/// \brief this is the central event loop that dispatches all requests. 
	/// 
	/// This all this class member functions are thread safe unless specified otherwise.
	///
	/// However only \b single thread may execute run() member function and dispatch its handlers, this class
	/// also can be safely created before fork and used after it
	///
	class BOOSTER_API io_service : public noncopyable, public io_events {
	public:

		///
		/// Create io service using specific reactor type and not default one (see reactor's class  use_* constants) 
		///
		io_service(int reactor_type);
		///
		/// Create io service using default reactor
		///
		io_service();
		///
		/// Destroy io_service. It should be stopped before!.
		///
		~io_service();

		///
		/// Set event handler \a h for file descriptor \a fd. \a event can be \ref io_events::in, \ref io_events::out 
		/// or \ref io_events::in | \ref io_events::out.
		///
		/// Error handling: 
		/// 
		/// - If invalid \a event type is given, std::invalid_argument is throw.
		/// - All other applicative errors are always reported using event handler \a h and never thrown directly.
		///
		void set_io_event(native_type fd,int event,event_handler const &h);
		///
		/// Cancel all io-events for file descriptor \a fd. Event handlers associated with this descriptor are
		/// dispatched asynchronously with aio_error::canceled error code.
		///
		void cancel_io_events(native_type fd);

		///
		/// Create a timer that expires in \a point time, that is handled with \a h handler.
		/// the handler will be called only in two cases:
		///
		/// - When the current time >= \a point.
		/// - Timer was canceled. \a h will be dispatched with aio_error::canceled error code.
		///
		/// The return value is special identification for the specific timer, it can be used for timer cancellation.
		/// Once cancel_timer_event is called with this unique identification, it should never be called again with
		/// this id as other timer may receive this identification.
		///
		int set_timer_event(ptime const &point,event_handler const &h);

		///
		/// Cancel timer created with set_timer_event() asynchronously,
		///
		void cancel_timer_event(int event_id);

		///
		/// Run main event loop. This function starts actual event loop. It does not return until stop
		/// is called or error occurs.
		///
		/// If error occurs (exception is thrown) you may try to restart the event loop by calling run once again.
		///
		/// However if run() is exited normally (i.e. by calling stop()) then you need to call reset() member function
		/// before next call of run().
		///
		void run();

		///
		/// Same as run(), but, event-loop specific errors are reported via \a e error code rather then by throwing.
		/// Note, event handlers still may throw.
		///
		void run(system::error_code &e);

		///
		/// Prepare the service after it was stopped. This function is not thread safe.
		///
		void reset();
		///
		/// Stop the service. All threads executing run() function will exit from it. You can't use this service
		/// till you call reset() function.
		///
		void stop();

		///
		/// Post a single handler \a h for immediate execution in the event loop queue. Useful for execution of some
		/// job in the thread that runs the event loop of the io_service.
		///
		void post(handler const &h);

		///
		/// Post event completion hander with its status
		///
		void post(event_handler const &h,booster::system::error_code const &e);
		///
		/// Post event i/o completion hander with its status and i/o size
		///
		void post(io_handler const &h,booster::system::error_code const &e,size_t n);


		///
		/// Get the real name of the reactor that io_service uses (calls reactor::name())
		///

		std::string reactor_name();
	private:
		struct data;
		hold_ptr<data> d;
		std::auto_ptr<event_loop_impl> impl_;	
	};

} // aio
} // booster

#endif
