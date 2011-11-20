//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_ENDPOINT_H
#define BOOSTER_AIO_ENDPOINT_H

#include <booster/config.h>
#include <booster/copy_ptr.h>
#include <booster/aio/types.h>
#include <string>

extern "C" {
	struct sockaddr;
};

namespace booster {
namespace aio {
	///
	/// \brief this class represents the connection endpoint, that is generally sockaddr structure 
	/// in Berkeley sockets API.
	///
	class BOOSTER_API endpoint {
	public:
		endpoint();
		endpoint(endpoint const &);
		endpoint const &operator = (endpoint const &);
		~endpoint();
		
		///
		/// Create a new IP endpoint using \a ip address and \a port
		///
		/// Throws system::system_error if the port or the address are invalid or not supported
		///
		endpoint(std::string const &ip,int port);

		///
		/// Set an IP address
		///
		/// Throws system::system_error if the address is invalid or not supported
		///
		void ip(std::string const &);
		///
		/// Set a port
		///
		/// Throws system::system_error if the port is invalid 
		///
		void port(int);

		///
		/// Get an ip
		///
		/// Throws system::system_error if the endpoint is not assigned or does not belong to IP protocol
		///
		std::string ip() const;
		///
		/// Get a port
		///
		/// Throws system::system_error if the endpoint is not assigned or does not belong to IP protocol
		///
		int port() const;

#ifndef BOOSTER_WIN32
		///
		/// Create a Unix domain socket endpoint,
		///
		/// Throws system::system_error if the path is not valid Unix domain socket path
		///
		endpoint(std::string const &);
		///
		/// Define a endpoint as a Unix domain socket endpoint
		///
		/// Throws system::system_error if the path is not valid Unix domain socket path
		///
		void path(std::string const &);
		///
		/// Get a endpoint path. 
		///
		/// Throws system::system_error if it is not assigned or the endpoint is not Unix domain
		/// socket endpoint
		///
		std::string path() const;
#endif

		///
		/// Get the endpoint family
		///
		/// Throws system::system_error if it is not assigned or the endpoint family is not supported
		///
		family_type family() const;
		
		///
		/// Set the native sockaddr structure as endpoint of size \a size
		///
		void raw(sockaddr const *p,int size);
		///
		/// The native sockaddr structure and its size type
		///
		typedef std::pair<sockaddr const *,int> native_address_type;
		///
		/// Get the native endpoint data structure
		///
		native_address_type raw() const;

	private:
		void throw_invalid() const;
		struct data;
		copy_ptr<data> d;
	};
} // aio
} // booster



#endif
