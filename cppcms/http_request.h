///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HTTP_REQUEST_H
#define CPPCMS_HTTP_REQUEST_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <booster/shared_ptr.h>
#include <cppcms/http_content_type.h>

#include <string>
#include <map>
#include <vector>

namespace cppcms {

namespace impl { namespace cgi { class connection; } }
namespace http {

	class cookie;
	class file;
	class content_limits;
	class content_filter;
	class basic_content_filter;

	///
	/// \brief This class represents all information related to the HTTP/CGI request.
	///
	/// It is usually accessed via http::context or application class
	///
	class CPPCMS_API request : public booster::noncopyable {
	public:

		// RFC 3875
		
		///
		/// CGI AUTH_TYPE environment variable
		///
		std::string auth_type();
		///
		/// CGI CONTENT_LENGTH environment variable
		///
		unsigned long long content_length();
		///
		/// CGI CONTENT_TYPE environment variable
		///
		std::string content_type();
		///
		/// Parsed CGI CONTENT_TYPE environment variable
		///
		cppcms::http::content_type content_type_parsed();
		///
		/// CGI GATEWAY_INTERFACE environment variable
		///
		std::string gateway_interface();
		///
		/// CGI PATH_INFO environment variable
		///
		std::string path_info();
		///
		/// CGI PATH_TRANSLATED environment variable
		///
		std::string path_translated();
		///
		/// CGI QUERY_STRING environment variable
		///
		std::string query_string();
		///
		/// CGI REMOTE_ADDR environment variable
		///
		std::string remote_addr();
		///
		/// CGI REMOTE_HOST environment variable
		///
		std::string remote_host();
		///
		/// CGI REMOTE_IDENT environment variable
		///
		std::string remote_ident();
		///
		/// CGI REMOTE_USER environment variable
		///
		std::string remote_user();
		///
		/// CGI REQUEST_METHOD environment variable
		///
		std::string request_method();
		///
		/// CGI SCRIPT_NAME environment variable
		///
		std::string script_name();
		///
		/// CGI SERVER_NAME environment variable
		///
		std::string server_name();
		///
		/// CGI SERVER_PORT environment variable
		///
		unsigned server_port();
		///
		/// CGI SERVER_PROTOCOL environment variable
		///
		std::string server_protocol();
		///
		/// CGI SERVER_SOFTWARE environment variable
		///
		std::string server_software();

		// RFC 2616 request headers
		
		///
		/// CGI HTTP_ACCEPT variable representing Accept HTTP header
		///
		std::string http_accept();
		///
		/// CGI HTTP_ACCEPT_CHARSET variable representing Accept-Charset HTTP header
		///
		std::string http_accept_charset();
		///
		/// CGI HTTP_ACCEPT_ENCODING variable representing Accept-Encoding HTTP header
		///
		std::string http_accept_encoding();
		///
		/// CGI HTTP_ACCEPT_LANGUAGE variable representing Accept-Language HTTP header
		///
		std::string http_accept_language();
		///
		/// CGI HTTP_ACCEPT_RANGES variable representing Accept-Ranges HTTP header
		///
		std::string http_accept_ranges();
		///
		/// CGI HTTP_AUTHORIZATION variable representing Authorization HTTP header
		///
		std::string http_authorization();
		///
		/// CGI HTTP_CACHE_CONTROL variable representing Cache-Control HTTP header
		///
		std::string http_cache_control();
		///
		/// CGI HTTP_CONNECTION variable representing Connection HTTP header
		///
		std::string http_connection();
		///
		/// CGI HTTP_COOKIE variable representing Cookie HTTP header
		///
		std::string http_cookie();
		///
		/// CGI HTTP_EXPECT variable representing Expect HTTP header
		///
		std::string http_expect();
		///
		/// CGI HTTP_FORM variable representing Form HTTP header
		///
		std::string http_form();
		///
		/// CGI HTTP_HOST variable representing Host HTTP header
		///
		std::string http_host();
		///
		/// CGI HTTP_IF_MATCH variable representing If-Match HTTP header
		///
		std::string http_if_match();
		///
		/// CGI HTTP_IF_NONE_MATCH variable representing If-None-Match HTTP header
		///
		std::string http_if_none_match();

		///
		/// GGI HTTP_MAX_FORWARDS variable representing Http-Max-Forwards
		///
		/// Returns a pair of {true,HTTP_MAX_FORWARDS} if set or {false,0} otherwise
		///
		std::pair<bool,unsigned> http_max_forwards();
		///
		/// CGI HTTP_PRAGMA variable representing Pragma HTTP header
		///
		std::string http_pragma();
		///
		/// CGI HTTP_PROXY_AUTHORIZATION variable representing Proxy-Authorization HTTP header
		///
		std::string http_proxy_authorization();
		///
		/// CGI HTTP_RANGE variable representing Range HTTP header
		///
		std::string http_range();
		///
		/// CGI HTTP_REFERER variable representing Referer HTTP header
		///
		std::string http_referer();
		///
		/// CGI HTTP_TE variable representing Te HTTP header
		///
		std::string http_te();
		///
		/// CGI HTTP_UPGRADE variable representing Upgrade HTTP header
		///
		std::string http_upgrade();
		///
		/// CGI HTTP_USER_AGENT variable representing User-Agent HTTP header
		///
		std::string http_user_agent();
		///
		/// CGI HTTP_VIA variable representing Via HTTP header
		///
		std::string http_via();
		///
		/// CGI HTTP_WARN variable representing Warn HTTP header
		///
		std::string http_warn();

		///
		/// Get CGI environment variable by name. Returns empty string if the variable is not set
		///
		std::string getenv(std::string const &);
		///
		/// Get CGI environment variable by name. Returns empty string if the variable is not set
		///
		std::string getenv(char const *);
		///
		/// Get CGI environment variable by name. Returns empty string if the variable is not set
		///
		char const *cgetenv(char const *);
		///
		/// Get map of all CGI environment variable as key-value pairs.
		///
		std::map<std::string,std::string> getenv();
	
			
		///
		/// Type that represents from-data key-value pairs
		///
		typedef std::multimap<std::string,std::string> form_type;
		///
		/// Type of a map of cookie name to the cookie value
		///
		typedef std::map<std::string,cookie> cookies_type;

		///
		/// Type that represents all files uploaded in this request
		///
		typedef std::vector<booster::shared_ptr<file> > files_type;

		///
		/// Get all cookies sent with this request
		///
		cookies_type const &cookies();
		///
		/// Get cookie by its name, if not assigned returns empty cookie
		///
		cookie const &cookie_by_name(std::string const &name);
		///
		/// Fetch GET value by name, if name not exists or more then one
		/// entry with same name exists, empty string is returned
		///
		std::string get(std::string const &name);
		///
		/// Fetch POST value by name, if name not exists or more then one
		/// entry with same name exists, empty string is returned
		///
		std::string post(std::string const &name);
		///
		/// form-data GET part of request
		///
		form_type const &get();
		///
		/// form-data POST part of request
		///
		form_type const &post();
		///
		/// form-data POST or GET according to reuqest_method()
		///
		form_type const &post_or_get();

		///
		/// Get all uploaded files
		///
		files_type files();
		
		///
		/// Access to raw bits of POST (content) data. If the content is empty if raw_content_filter is installed
		/// or multipart/form-data is handled the read_post_data().second will be 0;
		///
		/// Note: when processing multipart/form-data returns chunk of zero size as
		/// such requests maybe huge (file uploads of multiple hundreds of MB or even GB) that are would be stored in
		/// temporary files instead of memory. In order to get access to POST data you'll have to use post(), get(), or files()
		/// member functions.
		///
		std::pair<void *,size_t> raw_post_data();

		///
		///  Get content limits for incoming data processing
		///
		/// \ver{v1_2}
		content_limits &limits();

		///
		/// Get installed content filter, returns 0 if it is not installed, no ownership is transfered
		///
		/// \ver{v1_2}
		basic_content_filter *content_filter();
		
		///
		/// Installs content filter. If another filter installed it is removed
		///
		/// \ver{v1_2}
		void set_content_filter(basic_content_filter &flt);

		///
		/// Installs new content filter (or removes existing), the ownership of new filter is transfered to the request object
		///
		/// \ver{v1_2}
		void reset_content_filter(basic_content_filter *flt = 0);
		///
		/// Release existing content filter owned by request
		///
		/// \ver{v1_2}
		basic_content_filter *release_content_filter();
		///
		/// Returns true when full request content is ready
		///
		/// \ver{v1_2}
		bool is_ready();
		///
		/// Set the size of the buffer for content that isn't loaded to memory directly,
		/// like for example multipart/form-data, default is defined in configuration as
		/// service.input_buffer_size and defaults to 65536
		///
		/// \ver{v1_2}
		void setbuf(int size);
	public:
		/// \cond INTERNAL
		request(impl::cgi::connection &);
		~request();
		/// \endcond
	private:
		friend class context;
		friend class impl::cgi::connection;

		void set_filter(basic_content_filter *ptr,bool owns);

		int on_content_start();
		void on_error();
		int on_content_progress(size_t n);
		std::pair<char *,size_t > get_buffer();
	
		bool size_ok(file &f,long long size);

		std::pair<char *,size_t> get_content_buffer();
		bool prepare();
		bool parse_cookies();
		std::string urlencoded_decode(char const *,char const *);
		bool parse_form_urlencoded(char const *begin,char const *end,form_type &out);
		bool read_key_value(
				std::string::const_iterator &p,
				std::string::const_iterator e,
				std::string &key,
				std::string &value);

		struct _data;
		form_type get_;
		form_type post_;
		files_type files_;
		cookies_type cookies_;
		cppcms::http::content_type content_type_;
		booster::hold_ptr<_data> d;
		impl::cgi::connection *conn_;
	};


} // namespace http

} // namespace cppcms



#endif
