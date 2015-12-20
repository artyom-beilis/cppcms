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
#include <cppcms/http_content_filter.h>
#include "http_protocol.h"
#include <cppcms/util.h>
#include "cached_settings.h"
#include <cppcms/service.h>
#include "multipart_parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <booster/shared_ptr.h>


namespace cppcms { namespace http {
using cppcms::impl::multipart_parser;
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
	content_limits limits;
	basic_content_filter *filter;
	bool filter_owned;
	bool filter_is_raw_content_filter;
	bool filter_is_multipart_filter;
	bool ready;
	long long content_length;
	long long read_size;
	bool read_full;
	bool no_on_error;
	int buffer_size;
	booster::hold_ptr<cppcms::impl::multipart_parser> multipart_parser;	
	_data(cppcms::service &srv) : 
		limits(srv.cached_settings()),
		filter(0),
		filter_owned(false),
		filter_is_raw_content_filter(false), 
		filter_is_multipart_filter(false),
		ready(false),
		content_length(0),
		read_size(0),
		read_full(false),
		no_on_error(false),
		buffer_size(srv.cached_settings().service.input_buffer_size)
	{
	}
	~_data()
	{
		if(filter_owned)
			delete filter;
	}
};

void request::setbuf(int size)
{
	if(size < 1)
		size = 1;
	d->buffer_size = size;
}

basic_content_filter *request::content_filter()
{
	return d->filter;
}

void request::set_content_filter(basic_content_filter &flt)
{
	set_filter(&flt,false);
}
void request::reset_content_filter(basic_content_filter *flt)
{
	set_filter(flt,true);
}
basic_content_filter *request::release_content_filter()
{
	if(d->filter_owned) {
		basic_content_filter *flt = d->filter;
		d->filter = 0;
		d->filter_owned = false;
		return flt;
	}
	d->filter = 0;
	return 0;
}

void request::set_filter(basic_content_filter *filter,bool owned)
{
	if(d->filter && d->filter != filter && d->filter_owned) {
		delete d->filter;
		d->filter = 0;
	}
	d->filter = filter;
	d->filter_owned = filter ? owned : false;
	d->filter_is_multipart_filter   = dynamic_cast<multipart_filter   *>(d->filter) != 0;
	d->filter_is_raw_content_filter = dynamic_cast<raw_content_filter *>(d->filter) != 0;
}

bool request::is_ready()
{
	return d->ready;
}

std::pair<char *,size_t > request::get_buffer()
{
	char *ptr = 0;
	size_t size = 0;
	if(d->read_full) {
		ptr  = &d->post_data[0]     + d->read_size;
		size =  d->post_data.size() - d->read_size;
	}
	else {
		long long reminder = d->content_length - d->read_size;
		if(reminder < d->buffer_size)
			d->post_data.resize(reminder);
		else
			d->post_data.resize(d->buffer_size);
		if(d->post_data.size() == 0) {
			std::vector<char> tmp;
			tmp.swap(d->post_data);
		}
		else {
			ptr = &d->post_data[0];
			size = d->post_data.size();
		}
	}
	return std::make_pair(ptr,size);
}


bool request::size_ok(file &f,long long size)
{
	if(!f.has_mime() && f.size() > size) {
		BOOSTER_NOTICE("cppcms") << "multipart/form-data non-file entry size too big " << 
			f.size() 
			<< " REMOTE_ADDR = `" << getenv("REMOTE_ADDR") 
			<< "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
		return false;
	}
	return true;
}
namespace {
	std::string read_file(size_t reserve,std::istream &in)
	{
		std::string res;
		res.reserve(reserve);
		in.seekg(0);
		std::streambuf *buf = in.rdbuf();
		int c;
		while((c=buf->sbumpc())!=EOF) {
			res+=char(c);
		}
		return res;
	}
}


int request::on_content_progress(size_t n)
{
	if(n==0)
		return 0;
	char const *begin = &d->post_data[0];
	char const *end = begin + n;
	d->read_size += n;
	try {
		if(d->filter_is_raw_content_filter) {
			static_cast<raw_content_filter *>(d->filter)->on_data_chunk(&d->post_data[0],n);
		}

		if(d->multipart_parser.get()) {
			multipart_parser::parsing_result_type r = multipart_parser::continue_input;

			long long allowed=d->limits.content_length_limit();

			while(begin!=end) {
				r = d->multipart_parser->consume(begin,end);
				
				switch(r) { 
				case multipart_parser::meta_ready: 	
					{
						if(d->filter_is_multipart_filter) { 
							file &f=d->multipart_parser->get_file();
							static_cast<multipart_filter *>(d->filter)->on_new_file(f); 
						}
					}
					break;
				case multipart_parser::content_partial:
					{
						file &f=d->multipart_parser->get_file();
						if(!size_ok(f,allowed))
							return 413;
						if(d->filter_is_multipart_filter)
							 static_cast<multipart_filter *>(d->filter)->on_upload_progress(f);
					}
					break;
				case multipart_parser::content_ready:	
					{
						file &f=d->multipart_parser->last_file();
						f.data().seekg(0);
						if(!size_ok(f,allowed))
							return 413;
						if(d->filter_is_multipart_filter)
							 static_cast<multipart_filter *>(d->filter)->on_data_ready(f);
					}
					break;
				case multipart_parser::continue_input:
					break;
				case multipart_parser::no_room_left:
					return 413;
				case multipart_parser::eof:
					if(begin!=end) 
						return 400;
					if(d->read_size != d->content_length) 
						return 400;
					break;
				case multipart_parser::parsing_error:
				default:
					return 400;
				}
			}
			if(begin==end && d->read_size==d->content_length && r!=multipart_parser::eof) {
				return 400;
			}
		}

		if(d->read_size == d->content_length) {
			if(d->read_full) {
				if(content_type_.is_form_urlencoded()) {
					char const *data = &d->post_data[0];
					char const *data_end = data + d->post_data.size();
					parse_form_urlencoded(data,data_end,post_);
				}
			}
			else {
				std::vector<char> tmp;
				tmp.swap(d->post_data);
			}
			if(d->filter)
				d->filter->on_end_of_content();
			if(d->multipart_parser.get()) {
				multipart_parser::files_type mp=d->multipart_parser->get_files();
				d->multipart_parser.reset();
				
				for(multipart_parser::files_type::iterator p=mp.begin();p!=mp.end();++p) {
					multipart_parser::file_ptr f=*p;
					if(!f->has_mime()) 
						post_.insert(std::make_pair(f->name(),read_file(f->size(),f->data())));
					else 
						files_.push_back(f);
				}
			}
			d->ready = true;
		}
	}
	catch(abort_upload const &ab) {
		d->no_on_error=true;
		return ab.code();
	}
	catch(std::exception const &e) {
		BOOSTER_ERROR("cppcms") << e.what() << booster::trace(e);
		d->no_on_error=true;
		return 500;
	}
	catch(...) {
		BOOSTER_ERROR("cppcms") << "Unknown exception ";
		d->no_on_error=true;
		return 500;
	}
	return 0;
}

void request::on_error()
{
	if(d->no_on_error)
		return;
	if(d->filter) {
		d->filter->on_error();
	}
}

int request::on_content_start()
{
	if(d->content_length == 0)
		return 0;
	if(content_type_.is_multipart_form_data()) {
		if(d->content_length > d->limits.multipart_form_data_limit())
			return 413;
	}
	else {
		if(d->content_length > static_cast<long long>(d->limits.content_length_limit()))
			return 413;
	}
	if(!d->filter_is_raw_content_filter && !content_type_.is_multipart_form_data()) {
		d->post_data.resize(d->content_length);
		d->read_full = true;
	}
	else {
		if(content_type_.is_multipart_form_data() && !d->filter_is_raw_content_filter) {
			d->multipart_parser.reset(new multipart_parser(
				d->limits.uploads_path(),
				d->limits.file_in_memory_limit()));
			if(!d->multipart_parser->set_content_type(content_type_)) {
				return 400;
			}
		}
	}
	return 0;
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
	char const *s = conn_->cgetenv("CONTENT_LENGTH");
	if(!s || *s==0)
		d->content_length = 0;
	else
		d->content_length = atoll(s);
	content_type_ = cppcms::http::content_type(conn_->cgetenv("CONTENT_TYPE"));
	if(d->content_length == 0)
		d->ready = true;
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
	d(new _data(conn.service())),
	conn_(&conn)
{
}

content_limits &request::limits()
{
	return d->limits;
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
unsigned long long request::content_length() { return d->content_length; }
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
