///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HTTP_CLIENT_H
#define CPPCMS_HTTP_CLIENT_H

#include "defs.h"
#include "noncopyable.h"

namespace cppcms {
	class service;

	class http_client_impl;
	///
	/// Very simple HTTP client for performing HTTP requests from CppCMS applications
	///
	class CPPCMS_API http_client : public util::noncopyable {
	public:
		///
		/// Create new HTTP client
		///
		http_client(service &);	

		///
		/// Destructor
		///
		~http_client();

		///
		/// Set target URL that is requested
		///
		void url(std::string const &);
		///
		/// Set request method
		///
		void method(std::string const &);
		///
		/// Set POST data, calls method("POST") implicitly
		///
		void post_data(std::string const content_type,std::string const &data);

		///
		/// Add custom header, receives valid HTTP header without final CRLF 
		/// 
		void add_header(std::string const &)
		
		///
		/// Set timeout on request, default 10 seconds.
		///
		void timeout(int seconds);

		///
		/// Set response size limit (bytes), default 1MB
		///
		void response_size_limit(unsigned);

		typedef enum {
			ok,
			timeout,
			failed,
			invalid_request
		} completion_status_type;


		///
		/// Get HTTP status
		///
		int status();

		///
		/// Get HTTP reason
		///
		std::string reason();

		///
		/// Get response Body
		///
		std::string response();

		///
		/// Send request and get response synchronously
		///

		completion_status_type transfer();

		///
		/// Send request and get response asynchronously 
		///

		void async_transfer(booster::function<void(completion_status_type)> const &handler);

		///
		/// Reset the object to initial state for reuse
		///
		void reset();

		///
		/// Get reponse header by name, empty string returned in header does not exits
		///
		std::string get_header(std::string const &name);

		///
		/// Get all response headers
		///	
		std::vector<std::pair<std::string,std::string> > get_headers();

		///
		/// Get content type
		///
		std::string content_type();
		
	private:

		void check();

		booster::hold_ptr<http_client_impl> impl_;

	};
}



#endif
