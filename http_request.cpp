#include "http_request.h"

namespace cppcms { namespace http {

request::~request()
{
}

bool request::parse_cookies()
{
	std::string cookie=http_cookie();
	// TODO
	return true;	
}

std::string request::urlencoded_decode(char const *begin,char const *end)
{
	std::string result;
	result.reserve(end-begin);
	for(;begin<end;begin++) {
		char c=*begin;
		switch(c) {
		case '+': result+=' ';
		case '%':
			if(end-begin >= 3 && xdigit(begin[1]) && xdigit(begin[2])) {
				char buf[3]={begin[1],begin[2],0};
				int value;
				sscanf(buf,"%x",&value);
				result+=char(value);
			}
			
		}
	}
}

bool request::prepare()
{
	std::string query=query_string();
	if(!parse_form_urlencoded(query.data(),query.data()+query.size(),get_)) 
		return false;
	
	unsigned long long length=content_length();
	std::string content_type=content_type();
	if(is_prefix_of("application/x-www-form-urlencoded",content_type)) {
		if(length > 16*1024*1024) // Too Big??
			return false;
		std::vector<char> content(length,0);
		if(conn_->read(&content.front(),length)!=length)
			return false;
		if(!parse_from_urlencoded(&conent.front(),&content.front()+length,post_))
			return false;
	}
	else if(is_prefix_of("multipart/form-data",content_type)) {
		// not supported meanwhile
		// TODO
		return false;
	}
	if(!parse_cookies())
		return false;
	return true;
}

request::request(connection &conn) :
	conn_(conn)
{
	input_.reset(new input(conn));
}

request::~request()
{
}


///////////////////////
//
//


bool request::https() { return !conn->getenv("HTTPS").empty(); }
std::string request::server_software() { return conn_->getenv("SERVER_SOFTWARE"); }
std::string request::server_name() { return conn_->getenv("SERVER_NAME"); }
std::string request::gateway_interface(){ return conn_->getenv("GATEWAY_INTERFACE"); }
std::string request::server_protocol() { return conn_->getenv("SERVER_PROTOCOL"); }
unsigned request::server_port() { return atoi(conn_->getenv("SERVER_PORT")); }
std::string request::request_method() { return conn_->getenv("REQUEST_METHOD"); }
std::string request::path_info() { return conn_->getenv("PATH_INFO"); }
std::string request::path_translated() { return conn_->getenv("PATH_TRANSLATED"); }
std::string request::script_name() { return conn_->getenv("SCRIPT_NAME"); }
std::string request::script_filename() { return conn->getenv("SCRIPT_FILE_NAME"); }
std::string request::server_admin() { retrurn conn_->getenv("SERVER_ADMIN"); }
std::string request::server_signature() { return conn_->getenv("SERVER_SIGNATURE"); }
std::string request::query_string() { return conn_->getenv("QUERY_STRING"); }
std::string request::remote_host() { return conn_->getenv("REMOTE_HOST"); }
std::string request::remote_addr() { return conn_->getenv("REMOTE_ADDR"); }
std::string request::auth_type() { return conn_->getenv("AUTH_TYPE"); }
std::string request::remote_user() { return conn_->getenv("REMOTE_USER"); }
std::string request::remote_ident() { return conn_->getenv("REMOTE_IDENT"); }
std::string request::request_uri() { return conn_->getenv("REQUEST_URI"); }
std::string request::content_type() { return conn_->getenv("CONTENT_TYPE"); }
unsigned long long request::content_length() { return atoll(conn_->getenv("CONTENT_LENGTH")); }
std::string request::http_accept() { return conn_->getenv("HTTP_ACCEPT"); }
std::string request::http_user_agent() { return conn_->getenv("HTTP_USER_AGENT"); }
std::string request::http_accept_encoding() { return conn_->getenv("HTTP_ACCEPT_ENCODING"); }
std::string request::http_accept_language() { return conn_->getenv("HTTP_ACCEPT_LANGUAGE"); }
std::string request::document_root() { return conn_->getenv("DOCUMENT_ROOT"); }
std::string request::http_cookie() { return conn_->getenv("HTTP_COOKIE"); }
std::string request::http_forwarded() { return conn_->getenv("HTTP_FORWARDED"); }
std::string request::http_host() { return conn_->getenv("HTTP_HOST"); }
std::string request::http_forwarder()  { return conn_->getenv("HTTP_FORWARDER"); }
std::string request::http_referrer()  { return conn_->getenv("HTTP_REFERER"); }

std::string request::getenv(std::string const &)
{
	return conn_->getenv();
}
std::map<std::string,cookie> const &request::cookies()
{
	return cookies_;
}
		
form_type const &request::get()
{
	return get_;
}
form_type const &request::post()
{
	return post_;
}
std::vector<file> const &request::files()
{
	return files_;
}


///
/////////////////////////
//














} } // cppcms::http
