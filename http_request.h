#ifndef CPPCMS_HTTP_REQUEST_H
#define CPPCMS_HTTP_REQUEST_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"

#include <string>
#include <map>
#include <vector>

namespace cppcms {

namespace impl { namespace cgi { class connection; } }
namespace http {

	class cookie;
	class file;

	class CPPCMS_API request : public util::noncopyable {
	public:

		// RFC 3875
		std::string auth_type();
		unsigned long long content_length();
		std::string content_type();
		std::string gateway_interface();
		std::string path_info();
		std::string path_translated();
		std::string query_string();
		std::string remote_addr();
		std::string remote_host();
		std::string remote_ident();
		std::string remote_user();
		std::string request_method();
		std::string script_name();
		std::string server_name();
		unsigned server_port();
		std::string server_protocol();
		std::string server_software();

		// RFC 2616 request headers
		std::string http_accept();
		std::string http_accept_charset();
		std::string http_accept_encoding();
		std::string http_accept_language();
		std::string http_accept_ranges();
		std::string http_authrization();
		std::string http_cache_control();
		std::string http_connection();
		std::string http_cookie();
//		std::pair<bool,time_t> http_date();
		std::string http_expect();
		std::string http_form();
		std::string http_host();
		std::string http_if_match();
//		std::pair<bool,time_t> http_if_modified_since();
		std::string http_if_none_match();
//		std::pair<bool,time_t> http_if_unmodified_since();
		std::pair<bool,unsigned> http_max_forwards();
		std::string http_pragma();
		std::string http_proxy_authorization();
		std::string http_range();
		std::string http_referer();
		std::string http_te();
		std::string http_upgrade();
		std::string http_user_agent();
		std::string http_via();
		std::string http_warn();

		// Other
		std::string getenv(std::string const &);
		
		typedef std::multimap<std::string,std::string> form_type;
		typedef std::map<std::string,cookie> cookies_type;
		typedef std::vector<file *> files_type;

		cookies_type const &cookies();
		form_type const &get();
		form_type const &post();
		form_type const &post_or_get();
		files_type files();
		
		std::pair<void *,size_t> raw_post_data();

	public:
		request(impl::cgi::connection &);
		~request();
	private:

		friend class impl::cgi::connection;

		void set_post_data(std::vector<char> &post_data);
		bool prepare();

		bool parse_cookies();
		std::string urlencoded_decode(char const *,char const *);
		bool parse_form_urlencoded(char const *begin,char const *end,form_type &out);
		bool read_key_value(
				std::string::const_iterator &p,
				std::string::const_iterator e,
				std::string &key,
				std::string &value);

		struct data;
		form_type get_;
		form_type post_;
		files_type files_;
		cookies_type cookies_;
		util::hold_ptr<data> d;
		impl::cgi::connection *conn_;
	};


} // namespace http

} // namespace cppcms



#endif
