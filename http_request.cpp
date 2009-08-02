#include "cgi_api.h"
#include "http_request.h"
#include "http_cookie.h"
#include "http_file.h"

#include <stdio.h>
#include <boost/shared_ptr.hpp>

namespace cppcms { namespace http {

bool request::parse_cookies()
{
	std::string cookie=http_cookie();
	// TODO
	return true;	
}

namespace {
	bool inline xdigit(int c) { return ('0'<=c && c<='9') || ('a'<=c && c<='f') || ('A'<=c && c<='F'); }
	char ascii_to_lower(char c)
	{
		if('A'<=c && c<='Z')
			return 'a'+(c-'A');
		return c;
	}
	bool is_prefix_of(char const *prefix,std::string const &s)
	{
		size_t len=strlen(prefix);
		if(s.size() < len)
			return false;
		for(size_t i=0;i<len;i++) {
			if(ascii_to_lower(prefix[i]!=ascii_to_lower(s[i])))
				return false;
		}
		return true;
	}
}

struct request::data {
	std::vector<boost::shared_ptr<file> > files;
	std::vector<char> post_data;
};

void request::set_post_data(std::vector<char> &post_data)
{
	d->post_data.clear();
	d->post_data.swap(post_data);
}


// TODO: Find correct RFC for proprer decoding
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
	return result;
}

bool request::parse_form_urlencoded(char const *begin,char const *end,form_type &out)
{
	char const *p;
	for(p=begin;p<end;) {
		char const *e=std::find(p,end,'&');
		char const *name_end=std::find(p,e,'=');
		if(name_end==e || name_end==p)
			return false;
		std::string name=urlencoded_decode(p,name_end);
		std::string value=urlencoded_decode(name_end+1,e);
		out.insert(std::make_pair(name,value));
		p=e;
	}
	return true;
}

bool request::prepare()
{
	std::string query=query_string();
	if(!parse_form_urlencoded(query.data(),query.data()+query.size(),get_)) 
		return false;
	
	if(is_prefix_of("application/x-www-form-urlencoded",content_type())) {
		char const *pdata=&d->post_data.front();
		if(!parse_form_urlencoded(pdata,pdata+d->post_data.size(),post_)) 
			return false;
	}
	if(!parse_cookies())
		return false;
	return true;
}

request::request(impl::cgi::connection &conn) :
	d(new data),
	conn_(&conn)
{
}

request::~request()
{
}


///////////////////////
//
//


std::string request::auth_type() { return conn_->getenv("AUTH_TYPE"); }
unsigned long long request::content_length() { return atoll(conn_->getenv("CONTENT_LENGTH").c_str()); }
std::string request::content_type() { return conn_->getenv("CONTENT_TYPE"); }
std::string request::gateway_interface(){ return conn_->getenv("GATEWAY_INTERFACE"); }
std::string request::path_info() { return conn_->getenv("PATH_INFO"); }
std::string request::path_translated() { return conn_->getenv("PATH_TRANSLATED"); }
std::string request::query_string() { return conn_->getenv("QUERY_STRING"); }
std::string request::remote_addr() { return conn_->getenv("REMOTE_ADDR"); }
std::string request::remote_host() { return conn_->getenv("REMOTE_HOST"); }
std::string request::remote_ident() { return conn_->getenv("REMOTE_IDENT"); }
std::string request::remote_user() { return conn_->getenv("REMOTE_USER"); }
std::string request::request_method() { return conn_->getenv("REQUEST_METHOD"); }
std::string request::script_name() { return conn_->getenv("SCRIPT_NAME"); }
std::string request::server_name() { return conn_->getenv("SERVER_NAME"); }
unsigned request::server_port() { return atoi(conn_->getenv("SERVER_PORT").c_str()); }
std::string request::server_protocol() { return conn_->getenv("SERVER_PROTOCOL"); }
std::string request::server_software() { return conn_->getenv("SERVER_SOFTWARE"); }

std::string request::http_accept() { return conn_->getenv("HTTP_ACCEPT"); }
std::string request::http_accept_charset() { return conn_->getenv("HTTP_ACCEPT_CHARSET"); }
std::string request::http_accept_encoding() { return conn_->getenv("HTTP_ACCEPT_ENCODING"); }
std::string request::http_accept_language() { return conn_->getenv("HTTP_ACCEPT_LANGUAGE"); }
std::string request::http_accept_ranges() { return conn_->getenv("HTTP_ACCEPT_RANGES"); }

std::string request::http_authrization() { return conn_->getenv("HTTP_AUTHRIZATION"); }
std::string request::http_cache_control() { return conn_->getenv("HTTP_CACHE_CONTROL"); }
std::string request::http_connection() { return conn_->getenv("HTTP_CONNECTION"); }
std::string request::http_cookie() { return conn_->getenv("HTTP_COOKIE"); }
//time_t request::http_date() { return conn_->getenv("HTTP_DATE"); }
std::string request::http_expect() { return conn_->getenv("HTTP_EXPECT"); }
std::string request::http_form() { return conn_->getenv("HTTP_FORM"); }
std::string request::http_host() { return conn_->getenv("HTTP_HOST"); }
std::string request::http_if_match() { return conn_->getenv("HTTP_IF_MATCH"); }
//time_t request::http_if_modified_since() { return conn_->getenv("HTTP_IF_MODIFIED_SINCE"); }
std::string request::http_if_none_match() { return conn_->getenv("HTTP_IF_NONE_MATCH"); }
//time_t request::http_if_unmodified_since() { return conn_->getenv("HTTP_IF_UNMODIFIED_SINCE"); }
std::pair<bool,unsigned> request::http_max_forwards()
{
	typedef std::pair<bool,unsigned> res_type;
	std::string tmp=conn_->getenv("HTTP_MAX_FORWARDS");
	if(tmp.empty())
		return res_type(false,0);
	return res_type(true,atoi(tmp.c_str()));
}
std::string request::http_pragma() { return conn_->getenv("HTTP_PRAGMA"); }
std::string request::http_proxy_authorization() { return conn_->getenv("HTTP_PROXY_AUTHORIZATION"); }
std::string request::http_range() { return conn_->getenv("HTTP_RANGE"); }
std::string request::http_referer() { return conn_->getenv("HTTP_REFERER"); }
std::string request::http_te() { return conn_->getenv("HTTP_TE"); }
std::string request::http_upgrade() { return conn_->getenv("HTTP_UPGRADE"); }
std::string request::http_user_agent() { return conn_->getenv("HTTP_USER_AGENT"); }
std::string request::http_via() { return conn_->getenv("HTTP_VIA"); }
std::string request::http_warn() { return conn_->getenv("HTTP_WARN"); }

std::string request::getenv(std::string const &s)
{
	return conn_->getenv(s.c_str());
}
std::map<std::string,cookie> const &request::cookies()
{
	return cookies_;
}
		
request::form_type const &request::get()
{
	return get_;
}
request::form_type const &request::post()
{
	return post_;
}
request::files_type request::files()
{
	files_type files(d->files.size());
	for(unsigned i=0;i<d->files.size();i++) 
		files[i]=d->files[i].get();
	return files;
}


///
/////////////////////////
//














} } // cppcms::http
