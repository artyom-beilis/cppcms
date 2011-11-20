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
#include <booster/callback.h>
#include <booster/noncopyable.h>

namespace booster {
namespace aio {
	class io_service;
	class endpoint;

	///
	/// \brief This is a basic object that allows execution of asynchronous operations.
	///
	/// It represents a generic "select"able file descriptor or SOCKET on Windows platform.
	///
	/// It does following:
	///
	/// - Connects an object with the event loop - \ref io_service
	/// - Allows to connect and disconnect native file descriptor/socket object to it,
	///   and close it
	/// - Asynchronously poll the object for readability and writeability via \ref io_service object
	///   and cancel such operations
	/// - Switch blocking and non-blocking mode of the descriptor. 
	/// 
	///
	class BOOSTER_API basic_io_device : public noncopyable {
	public:
		///
		/// Create a new device not attached to event loop
		///
		basic_io_device();
		///
		/// Create a new device that is attached to the event loop \a srv
		///
		basic_io_device(io_service &srv);
		///
		/// Destroy the object. If it owns the file descriptor or socket it closes it
		///
		virtual ~basic_io_device();

		///
		/// Check if the basic_io_device is connected to the io_service
		///
		bool has_io_service();
		///
		/// Returns the connected io_service, throws system::system_error if no \ref io_service connected
		///
		io_service &get_io_service();
		///
		/// Sets new io_service. Cancels all pending asynchronous operations on the connected io_service.
		///
		void set_io_service(io_service &srv);
		///
		/// Detaches the object from io_service.  Cancels all pending asynchronous operations.
		///
		void reset_io_service();

		///
		/// Attach the file descriptor \a fd to the device. The ownership is not transferred to the object.
		///
		/// If basic_io_device owns other file descriptor, it is closed.
		///
		void attach(native_type fd);
		///
		/// Assign existing file descriptor \a fd to the device. The ownership is transferred to the object
		///
		void assign(native_type fd);
		///
		/// Release the ownership on the current file descriptor. The user is responsible to close it.
		///
		/// \note it just changes the "ownership" flag in the object, nothing else is done
		///
		native_type release();
		///
		/// Get the underlying file descriptor. Returns invalid_socket if the file descriptor was not assigned
		///
		native_type native();
	
		///
		/// Cancels all pending asynchronous events. If the ownership belongs to it closes the file descriptor.
		///
		/// Throws system::system_error if error occurs.
		///	
		void close();
		///
		/// Cancels all pending asynchronous events. If the ownership belongs to it closes the file descriptor.
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void close(system::error_code &e);

		///
		/// Start asynchronous polling for readability. The result is reported via callback \a r.
		///
		/// If io_service is not assigned throws system::system_error, all other errors reported via \a r.
		///
		void on_readable(event_handler const &r);
		///
		/// Start asynchronous polling for writeability. The result is reported via callback \a r.
		///
		/// If io_service is not assigned throws system::system_error, all other errors reported via \a r.
		///
		void on_writeable(event_handler const &r);
		///
		/// Cancel all asynchronous operations.
		///
		void cancel();

		///
		/// Returns *this
		///
		basic_io_device &lowest_layer();

		///
		/// Set the object to blocking or non-blocking mode.
		///
		/// Throws system::system_error if error occurs.
		///	
		void set_non_blocking(bool nonblocking);
		///
		/// Set the object to blocking or non-blocking mode.
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void set_non_blocking(bool nonblocking,system::error_code &e);

		///
		/// Check if a error code \a e reports that the non-blocking operation would block
		///
		static bool would_block(system::error_code const &e);

	protected:
		///
		/// Set non-blocking mode. If error occurs returns false and the error is reported via callback c
		///
		bool dont_block(event_handler const &c);
		///
		/// Set non-blocking mode. If error occurs returns false and the error is reported via callback c
		///
		bool dont_block(io_handler const &c);
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
