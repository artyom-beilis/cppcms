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

	///
	/// \brief this class represents a socket that accepts incoming connections
	///	
	class BOOSTER_API acceptor : public basic_socket {
	public:

		///
		/// Create a new acceptor object
		///
		acceptor();
		///
		/// Create a new acceptor object with assigned \ref io_service  \a srv)
		///
		acceptor(io_service &srv);
		~acceptor();

		///
		/// Opens a new stream socket of a \ref family_type \a d
		///
		/// Throws system::system_error if error occurs.
		///	
		void open(family_type d);
		///
		/// Opens a new stream socket of a \ref family_type \a d
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void open(family_type d,system::error_code &e);

		///
		/// Accepts a new incoming connection to the socket \a s
		///
		/// Throws system::system_error if error occurs.
		///	
		void accept(stream_socket &s);
		///
		/// Accepts a new incoming connection to the socket \a s
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void accept(stream_socket &s,system::error_code &e);

		///
		/// Bind the opended socket the \ref endpoint \a ep
		///
		/// Throws system::system_error if error occurs.
		///	
		void bind(endpoint const &ep);
		///
		/// Bind the opended socket the \ref endpoint \a ep
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void bind(endpoint const &ep,system::error_code &e);
		///
		/// Starts listening on the socket with backlog parameter \a backlog
		///
		/// Throws system::system_error if error occurs.
		///	
		void listen(int backlog);
		///
		/// Starts listening on the socket with backlog parameter \a backlog
		///
		/// If a error occurs it is assigned to \a e.
		///	
		void listen(int backlog,system::error_code &e);

		///
		/// Accept the connection asynchronously. The reference \a s must be valid until h is called.
		///
		/// If io_service is not assigned throws system::system_error, all other errors reported via 
		/// the callback \a h.
		///
		void async_accept(stream_socket &s,event_handler const &h);
	
	private:
		struct data;
		hold_ptr<data> d;
	};
	
} // aio
} // booster

#endif
