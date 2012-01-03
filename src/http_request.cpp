///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "cgi_api.h"
#include <cppcms/http_request.h>
#include <cppcms/http_cookie.h>
#include <cppcms/http_file.h>
#include "http_protocol.h"
#include <cppcms/util.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <booster/shared_ptr.h>


namespace cppcms { namespace http {

namespace {
	template<typename Iterator>
	void skip_after_period(Iterator &p,Iterator e)
	{
		while(p<e) {
			if(*p==';' || *p==',') {
				++p;
				return;
			}
			++p;
		}
	}
}

bool request::read_key_value(
		std::string::const_iterator &p,
		std::string::const_iterator e,
		std::string &key,
		std::string &value)
{
	std::string::const_iterator tmp;
	p=http::protocol::skip_ws(p,e);
	tmp=p;
	p=http::protocol::tocken(p,e);
	if(p==tmp && p<e) {
		skip_after_period(p,e);
		return false;
	}
	key.assign(tmp,p);
	p=http::protocol::skip_ws(p,e);
	if(p<e) {
		if(*p!='=') {
			if(*p==';' || *p==',') {
				++p;
				return true;
			}
		}
	}
	else {
		return true;
	}
	p=http::protocol::skip_ws(p+1,e);
	if(p < e) {
		if(*p=='"') {
			tmp=p;
			value=http::protocol::unquote(p,e);
			if(p==tmp) {
				p=e;
				return false;
			}
		}
		else {
			tmp=p;
			p=http::protocol::tocken(p,e);
			value.assign(tmp,p);
			if(p==tmp && p<e && *p!=';'&& *p!=',') {
				skip_after_period(p,e);
				return false;
			}
		}
	}
	else {
		return true;
	}
	p=http::protocol::skip_ws(p,e);
	if(p<e && (*p==';' || *p==',' ))
		p=http::protocol::skip_ws(p+1,e);
	return true;
}

bool request::parse_cookies()
{
	std::string const cookie_str=http_cookie();
	std::string::const_iterator p=cookie_str.begin(),e=cookie_str.end();
	p=http::protocol::skip_ws(p,e);
	http::cookie cookie;
	while(p<e) {
		std::string key,value;
		if(!read_key_value(p,e,key,value)) {
			cookie = http::cookie();
			continue;
		}
		if(key[0]=='$') {
			if(cookie.name().empty()) {
				if(http::protocol::compare(key,"$Path")==0)
					cookie.name(value);
				else if(http::protocol::compare(key,"$Domain")==0)
					cookie.domain(value);
			}
		}
		else {
			if(!cookie.name().empty())
				cookies_.insert(std::make_pair(cookie.name(),cookie));
			cookie=http::cookie(key,value);
		}
	}
	if(!cookie.name().empty())
		cookies_.insert(std::make_pair(cookie.name(),cookie));
	return true;	
}


struct request::_data {
	std::vector<char> post_data;
};

void request::set_post_data(std::vector<char> &post_data)
{
	d->post_data.clear();
	d->post_data.swap(post_data);

	if(content_type_.media_type() == "application/x-www-form-urlencoded") {
		if(!d->post_data.empty()) {
			char const *pdata=&d->post_data.front();
			parse_form_urlencoded(pdata,pdata+d->post_data.size(),post_);
		}
	}
}

namespace {
	std::string read_file(std::istream &in)
	{
		std::string res;
		while(in.good() && !in.eof()) {
			char buf[256];
			in.read(buf,256);
			res.append(buf,in.gcount());
		}
		return res;
	}
}

void request::set_post_data(std::vector<booster::shared_ptr<file> > const &multipart)
{
	for(unsigned i=0;i<multipart.size();i++) {
		if(multipart[i]->mime().empty() || multipart[i]->filename().empty()) {
			post_.insert(std::make_pair(multipart[i]->name(),read_file(multipart[i]->data())));
		}
		else {
			files_.push_back(multipart[i]);
		}
	}
}


bool request::parse_form_urlencoded(char const *begin,char const *end,form_type &out)
{
	char const *p;
	for(p=begin;p<end;) {
		char const *e=std::find(p,end,'&');
		char const *name_end=std::find(p,e,'=');
		if(name_end==e || name_end==p)
			return false;
		std::string name=util::urldecode(p,name_end);
		std::string value=util::urldecode(name_end+1,e);
		out.insert(std::make_pair(name,value));
		p=e+1;
	}
	return true;
}

bool request::prepare()
{
	char const *query=cgetenv("QUERY_STRING");
	if(!parse_form_urlencoded(query,query + strlen(query),get_))  {
		get_.clear();
	}
	parse_cookies();
	content_type_ = cppcms::http::content_type(conn_->cgetenv("CONTENT_TYPE"));
	return true;
}

std::pair<void *,size_t> request::raw_post_data()
{
	std::pair<void *,size_t> r;
	static char b;
	if(d->post_data.empty()) {
		r.first=&b;
		r.second=0;
	}
	else {
		r.first=&d->post_data.front();
		r.second=d->post_data.size();
	}
	return r;
}

request::request(impl::cgi::connection &conn) :
	d(new _data),
	conn_(&conn)
{
}

request::~request()
{
}


///////////////////////
//
//


cppcms::http::content_type request::content_type_parsed()
{
	return content_type_;
}

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

std::string request::http_authorization() { return conn_->getenv("HTTP_AUTHORIZATION"); }
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


std::map<std::string,std::string> request::getenv()
{
	return conn_->getenv();
}
std::string request::getenv(char const *s)
{
	return conn_->getenv(s);
}
char const *request::cgetenv(char const *s)
{
	return conn_->cgetenv(s);
}
std::map<std::string,cookie> const &request::cookies()
{
	return cookies_;
}

static cookie const empty_cookie;

cookie const &request::cookie_by_name(std::string const &name)
{
	cookies_type::const_iterator p = cookies_.find(name);
	if(p==cookies_.end())
		return empty_cookie;
	else
		return p->second;
}

std::string request::get(std::string const &name)
{
	std::pair<form_type::iterator,form_type::iterator> pair = get_.equal_range(name);
	form_type::iterator p = pair.first;
	if(pair.first == pair.second || ++pair.first != pair.second)
		return std::string();
	return p->second;
}

std::string request::post(std::string const &name)
{
	std::pair<form_type::iterator,form_type::iterator> pair = post_.equal_range(name);
	form_type::iterator p = pair.first;
	if(pair.first == pair.second || ++pair.first != pair.second)
		return std::string();
	return p->second;
}
		
request::form_type const &request::get()
{
	return get_;
}
request::form_type const &request::post()
{
	return post_;
}
request::form_type const &request::post_or_get()
{
	if(request_method()=="POST")
		return post_;
	else
		return get_;
}
request::files_type request::files()
{
	return files_;
}


///
/////////////////////////
//














} } // cppcms::http
