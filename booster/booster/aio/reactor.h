//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_REACTOR_H
#define BOOSTER_AIO_REACTOR_H
#include <booster/config.h>
#include <booster/aio/types.h>
#include <memory>
#include <string>
namespace booster {
namespace aio {

	class reactor_impl;

	///
	/// \brief This class is an abstraction of platform dependent polling API.
	/// 
	/// It abstracts platform specific APIs like epoll, /dev/poll, kqueue, poll
	/// and select.
	///
	/// It provides platform indepenent functionality for polling file descriptions
	/// in efficient way
	///
	class BOOSTER_API reactor : public io_events {
		reactor(reactor const &);
		void operator=(reactor const &);
	public:

		static const int use_default	= 0; //< Best polling device available on the OS
		static const int use_select	= 1; //< select() API, default on Windows
		static const int use_poll	= 2; //< poll() API default on POSIX platforms without better polling support
		static const int use_epoll	= 3; //< epoll() available and default on Linux
		static const int use_dev_poll	= 4; //< /dev/poll available and default on Solaris and HP-UX
		static const int use_kqueue	= 5; //< kqueue available and default on BSD and Mac OS X.
		static const int use_max	= use_kqueue;  //< Maximal number

		/// \brief structure that defines output events
		struct event {
			native_type fd; //< A descriptor that event occurred on
			int events;     //< a bit mask of events \see io_events
		};


		///
		/// Create a new polling device using \a hint recommendation. If provided
		/// device is not supported the default is used
		///
		reactor(int hint = use_default);
		~reactor();

		///
		/// Add \a fd to watch list, flags can be a or mask of \ref io_events::in, \ref io_events::out
		/// and \ref io_events::err
		///
		/// If flags=0 removes the \a fd from the watch list
		///
		/// If error occurs throws \ref system::system_error
		///
		void select(native_type fd,int flags);
		///
		/// Add \a fd to watch list, flags can be a or mask of \ref io_events::in,
		/// \ref io_events::out and \ref io_events::err
		///
		/// If flags=0 removes the \a fd from the watch list
		///
		/// If error occurs, the error code is assigned to \a e
		///
		void select(native_type fd,int flags,system::error_code &e);
		///
		/// Removes the \a  fd from the watch list
		///
		/// If error occurs throws \ref system::system_error
		///
		void remove(native_type fd)
		{
			select(fd,0);
		}
		///
		/// Removes the \a fd from the watch list
		///
		/// If error occurs, the error code is assigned to \a e
		///
		void remove(native_type fd,system::error_code &e)
		{
			select(fd,0,e);
		}
		
		///
		/// Poll for the events, wait at most \a timeout microseconds. 
		/// Save up to \a n values to the \a events vector.
		///
		/// Returns number of events recorded. Returns 0 if timeout occurred.
		/// 
		/// If error occurs throws \ref system::system_error
		///
		int poll(event *events,int n,int timeout);
		///
		/// Poll for the events, wait at most \a timeout microseconds. 
		/// Save up to \a n values to the \a events vector.
		///
		/// Returns number of events recorded. Returns 0 if timeout occurred.
		/// 
		/// If error occurs, the error code is assigned to \a e
		///
		int poll(event *events,int n,int timeout,system::error_code &e);

		///
		/// Get the actual name of the polling device used.
		///
		std::string name() const;
	private:
		std::auto_ptr<reactor_impl> impl_;
	};
} // aio
} // booster


#endif
