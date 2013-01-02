///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HTTP_RESPONSE_H
#define CPPCMS_HTTP_RESPONSE_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>

#include <string>
#include <iostream>
#include <cppcms/cstdint.h>

namespace cppcms {
class cache_interface;
namespace impl { namespace cgi { class connection;  }}
namespace http {

	class context;
	class cookie;

	///
	/// \brief this class represents all HTTP/CGI response related API, generation of output content and
	/// HTTP headers.
	///
	class CPPCMS_API response : public booster::noncopyable {
	public:
		///
		/// This type represent the enum of HTTP Status type as of RFC 2616 sec. 6.1.1
		///
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

		///
		/// This enum represents different types of IO modes giving fine grained control over HTTP output.
		///
		/// These modes are set via io_mode member function
		///
		typedef enum {
			normal, ///< Synchronous IO. Write the request, it is buffered and possible compressed using gzip.
			nogzip, ///< Same as normal but disable gzip compression
			raw,    ///< User has full control over the output, it is also fully responsible for generation
				///< of HTTP headers, no-session management given.
			asynchronous,
				///< In this mode the data is buffered and never transferred unless it is requested by async_flush_output,
				///< or async_complete_response member functions of http::context explicitly.
			asynchronous_raw
				///< Same as asynchronous but the user is responsible for generation of its own HTTP headers.

		} io_mode_type;


		// Standard HTTP Response Headers RFC 2616

		///
		/// Set HTTP Header Accept-Ranges
		///
		void accept_ranges(std::string const &);
		///
		/// Set HTTP Header Age
		///
		void age(unsigned seconds);
		///
		/// Set HTTP Header Allow
		///
		void allow(std::string const &);
		///
		/// Set HTTP Header Cache-Control
		///
		void cache_control(std::string const &);
		///
		/// Set HTTP Header Content-Encoding.
		///
		/// Note: if you set this header manually, gzip compression will be automatically disabled.
		///
		void content_encoding(std::string const &);
		///
		/// Set HTTP Header Content-Language
		///
		void content_language(std::string const &);
		///
		/// Set HTTP Header Content-Length
		///
		void content_length(unsigned long long len);
		///
		/// Set HTTP Header Content-Location
		///
		void content_location(std::string const &);
		///
		/// Set HTTP Header Content-Md5
		///
		void content_md5(std::string const &);
		///
		/// Set HTTP Header Content-Range
		///
		void content_range(std::string const &);
		///
		/// Set HTTP Header Content-Type
		///
		void content_type(std::string const &);
		///
		/// Set HTTP Header Date
		///
		void date(time_t);
		///
		/// Set HTTP Header Etag
		///
		void etag(std::string const &);
		///
		/// Set HTTP Header Expires, it is formatted as HTTP Date-Time from the POSIX time \a t
		///
		void expires(time_t t);
		///
		/// Set HTTP Header Last-Modified, it is formatted as HTTP Date-Time from the POSIX time \a t
		///
		void last_modified(time_t t);
		///
		/// Set HTTP Header Location
		///
		void location(std::string const &);
		///
		/// Set HTTP Header Pragma
		///
		void pragma(std::string const &);
		///
		/// Set HTTP Header Proxy-Authenticate
		///
		void proxy_authenticate(std::string const &);
		///
		/// Set HTTP Header Retry-After
		///
		void retry_after(std::string const &);
		///
		/// Set HTTP Header Retry-After
		///
		void retry_after(unsigned);
		///
		/// Set GCI Status Header, the message is created according to the code automatically
		///
		void status(int code);
		///
		/// Set HTTP Header Status with the status code \a code and a custom \a message
		///
		void status(int code,std::string const &message);
		///
		/// Set HTTP Header Trailer
		///
		void trailer(std::string const &);
		///
		/// Set HTTP Header Transfer-Encoding, note
		///
		void transfer_encoding(std::string const &);
		///
		/// Set HTTP Header Vary
		///
		void vary(std::string const &);
		///
		/// Set HTTP Header Via
		///
		void via(std::string const &);
		///
		/// Set HTTP Header Warning
		///
		void warning(std::string const &);
		///
		/// Set HTTP Header WWW-Authenticate
		///
		void www_authenticate(std::string const &);


		///
		/// Set Generic HTTP header "name: value"
		///
		void set_header(std::string const &name,std::string const &value);
		///
		/// Get output header value, returns empty string if it is not set
		///
		std::string get_header(std::string const &name);
		///
		/// Erase specific header \a h
		///
		void erase_header(std::string const &h);

		///
		/// This function set HTTP Content-Type header, but unlike contet_type function it also adds
		/// current locale charset encoding (unless localization.disable_charset_in_content_type set to true in configuration)
		///
		void set_content_header(std::string const &content_type);

		///
		/// Shortcut to set_content_header("text/html") - this is the default content-type header
		///
		void set_html_header();
		///
		/// Shortcut to set_content_header("text/xhtml")
		///
		void set_xhtml_header();
		///
		/// Shortcut to set_content_header("text/plain")
		///
		void set_plain_text_header();
		///
		/// Redirect to other \a location by setting HTTP Location header and a Status header with a status \a status
		///
		void set_redirect_header(std::string const &location,int status = found);
		///
		/// Set HTTP Cookie
		///
		void set_cookie(cookie const &);
	
		///
		/// This creates a default HTTP response (including HTML body) with a status \a stat a message \a msg.
		///
		/// Note: if \a msg is empty default message for HTTP Status code \a stat is used.
		///	
		void make_error_response(int stat,std::string const &msg = std::string());

		///
		/// Get current io_mode, see a description for io_mode_type.
		///
		io_mode_type io_mode();
		///
		/// Set current io_mode, see a description for io_mode_type.
		///
		/// Note: you can't change it after requesting for output stream, i.e. calling out() member function.
		///
		void io_mode(io_mode_type);

		///
		/// Request an output stream to write the body of HTTP response after writing headers.
		///
		/// Note:
		///
		/// - it triggers saving changes in the session, so further changes will have no effect.
		/// - it triggers writing all headers to output stream, such that changing any header or adding cookies will have no effect.
		/// - it is impossible to change an io_mode after calling this function.
		///
		std::ostream &out();

		///
		/// Create an HTTP time string from POSIX time as of RFC 2616
		///
		static std::string make_http_time(time_t);
		///
		/// Return a string that represents a message for HTTP status code \a status
		///
		static char const *status_to_string(int status);

		///
		/// Check if out() member function was called, (so it is impossible to do some tasks like changing HTTP headers.
		/// See out description for more details
		///
		bool some_output_was_written();
		///
		/// Finalize an output request. Generally this function is called automatically for synchronous application, however
		/// when working with asynchronous requests you should call this function before calling async_complete_response.
		///
		void finalize();

		/// \cond INTERNAL 
		response(context &);
		~response();
		/// \endcond
	private:
		friend class impl::cgi::connection;
		friend class ::cppcms::cache_interface;

		void copy_to_cache();
		std::string copied_data();
		bool need_gzip();

		std::pair<char const *,size_t> output();

		void write_http_headers(std::ostream &);

		typedef std::pair<booster::shared_ptr<std::vector<char> >,size_t> chunk_type;
		chunk_type get_async_chunk();

		struct _data;
		booster::hold_ptr<_data> d;

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
