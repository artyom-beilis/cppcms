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
#ifndef CPPCMS_HTTP_RESPONSE_H
#define CPPCMS_HTTP_RESPONSE_H

#include "defs.h"
#include "noncopyable.h"
#include <booster/hold_ptr.h>

#include <string>
#include <iostream>
#include "cstdint.h"

namespace cppcms {
class cache_interface;
namespace impl { namespace cgi { class connection;  }}
namespace http {

	class context;
	class cookie;

	class CPPCMS_API response : public util::noncopyable {
	public:
		// RFC 2616 sec. 6.1.1
		typedef enum {
			continue_transfer = 100,
			switching_protocol = 101,
			ok = 200,
			created = 201,
			accepted = 202,
			non_authoritative_information = 203,
			no_content = 204,
			reset_content = 205,
			partial_content = 206,
			multiple_choices = 300,
			moved_permanently = 301,
			found = 302,
			see_other = 303,
			not_modified = 304,
			use_proxy = 305,
			temporary_redirect = 307,
			bad_request = 400,
			unauthorized = 401,
			payment_required = 402,
			forbidden = 403,
			not_found = 404,
			method_not_allowed = 405,
			not_acceptable = 406,
			proxy_authentication_required = 407,
			request_time_out = 408,
			conflict = 409,
			gone = 410,
			precondition_failed = 412,
			request_entity_too_large = 413,
			request_uri_too_large = 414,
			unsupported_media_type = 415,
			requested_range_not_satisfiable = 416,
			expectation_failed = 417,
			internal_server_error = 500,
			not_implemented = 501,
			bad_gateway = 502,
			service_unavailable = 503,
			gateway_timeout = 504,
			http_version_not_supported = 505
		} status_type;
		typedef enum {
		// synchronous io
			normal, // write request, use buffering, possible compression,
			nogzip, // as normal but disable gzip
			raw,    // user writes its own headers to stream directly
			asynchronous,
				// the data is buffered and never transferred
				// untill it is requested explicitly
			asynchronous_raw
				// the data is buffered and nevet transferred
				// untill it is requested explicitly, headers are not written

		} io_mode_type;


		// Standard HTTP Response Headers RFC 2616

		void accept_ranges(std::string const &);
		void age(unsigned seconds);
		void allow(std::string const &);
		void cache_control(std::string const &);
		void content_encoding(std::string const &);
		void content_language(std::string const &);
		void content_length(unsigned long long len);
		void content_location(std::string const &);
		void content_md5(std::string const &);
		void content_range(std::string const &);
		void content_type(std::string const &);
		void date(time_t);
		void etag(std::string const &);
		void expires(time_t);
		void last_modified(time_t);
		void location(std::string const &);
		void pragma(std::string const &);
		void proxy_authenticate(std::string const &);
		void retry_after(std::string const &);
		void retry_after(unsigned);
		void status(int code);
		void status(int code,std::string const &message);
		void trailer(std::string const &);
		void transfer_encoding(std::string const &);
		void vary(std::string const &);
		void via(std::string const &);
		void warning(std::string const &);
		void www_authenticate(std::string const &);


		void set_header(std::string const &name,std::string const &value);
		std::string get_header(std::string const &name);
		void erase_header(std::string const &);
		void clear();

		void set_content_header(std::string const &content_type);
		void set_html_header();
		void set_xhtml_header();
		void set_plain_text_header();
		void set_redirect_header(std::string const &location,int status = found);
		void set_cookie(cookie const &);
		
		void make_error_response(int stat,std::string const &msg = std::string());


		io_mode_type io_mode();
		void io_mode(io_mode_type);
		std::ostream &out();

		static std::string make_http_time(time_t);
		static char const *status_to_string(int status);

		bool some_output_was_written();
		void finalize();

		response(context &);
		~response();
	private:
		friend class impl::cgi::connection;
		friend class ::cppcms::cache_interface;

		void copy_to_cache();
		std::string copied_data();
		bool need_gzip();

		std::pair<char const *,size_t> output();

		void write_http_headers(std::ostream &);
		std::string get_async_chunk();

		struct data;
		booster::hold_ptr<data> d;

		context &context_;
		std::ostream *stream_;
		io_mode_type io_mode_;

		uint32_t disable_compression_ : 1;
		uint32_t ostream_requested_ : 1;
		uint32_t copy_to_cache_ : 1;
		uint32_t finalized_ : 1;
		uint32_t reserved_ : 28;
	};

} /* http */
} /* cppcms */


#endif
