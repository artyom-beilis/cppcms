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
#define CPPCMS_SOURCE
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include "http_protocol.h"
#include <cppcms/service.h>
#include "service_impl.h"
#include <cppcms/json.h>
#include "cgi_api.h"
#include <cppcms/util.h>

#include <stdlib.h>
#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

#include <booster/log.h>


namespace cppcms { namespace impl { namespace cgi {
/*
class multipart_separator {
public:
	multipart_separator(std::vector<char> &body,unsigned &body_ptr,std::string boundary) :
		body_(body),
		body_ptr_(body_ptr)
	{
		boundary_ = "--" + boundary;
		pos_ = 0;
		last_ = false;
	}
	
	int getc()
	{
		if(body_ptr_ < body_.size()) {
			return body_[body_ptr_++];
		}
		else {
			body_.clear();
			body_ptr_=0;
			return -1;
		}
	}

	enum { incomplete, last_chunk, chunk };
	int next()
	{
		for(;;){
			int c=getc();
			if(c < 0)
				return incomplete;
			if(pos_ < bodundary_.size()) {
				if(c == boundary_[pos_]) {
					pos_++;
				}
				else {
					if(pos_ != 0)
						output_.append(boundary_.substr(0,pos_));
					output_.append(char(c));
					pos_ = 0;
				}
			}
			else if(pos_ == boundary_.size()) {
				c == '-';
				last_ = true;
				pos_ = 0x10001;
			}
			else {
				unsigned diff = pos_ & 0xFFFF;
				if(last_){
					if(last_ending[diff]==c) {
						pos_ ++;
						diff ++ ;
					}
					else {
						output_.append(last_ending,diff);
						output_.append(char(c));
						pos_ = 0;
						last_ = false;
					}
					if(diff == 4)
						return last_chunk;
				}
				else {
					if(ending[diff] == c) {
						pos_ ++;
						diff ++;
					}
					else {
						output_.append(ending,diff);
						output_.append(char(c));
						pos_ = 0;
					}
					if(diff == 2) {
						pos_ = 0;
						return chunk;
					}
				}
			}
		}
	}

private:
	static char const last_ending[]="--\r\n"
	static char const ending[]="\r\n"
	
};


class multipart_parser : public booster::noncopyable {
public:
	multipart_parser(std::vector<char> &body, unsigned &ptr) :
		separator_(body,ptr),
		parser_(body,ptr)
	{
	}

	struct none{};
	struct eof{};
	typedef std::pair<std::string,std::string> pair_type;
	typedef booster::shared_ptr<http::file> file_type;
	typedef enum { none, eof, error } result_type;
	typedef boost::variant<result_type,pair_type,file_type,eof> variant_type;

	void append(bool final = false)
	{
		if(result_.which() == 1) {
			std::string &last=boost::get<pair_type>(result_).second;
			last.append(separator_.output());
		}
		else if(result_.which() == 2) {
			file_type &file == boost::get<file_type>(result_);
			file.write(separator_.output());
			if(final)
				file.close();
		}
	}

	variant_type next()
	{
		switch(state:) {
		case done:
			return eof;
		case reading_data:
			switch(separator_.next()) {
			case multipart_separator::incomplete:
				append();
				return none;
			case multipart_separator::chunk;
				{
					append(final);
					variant_type tmp = result_;
					result_=none;
					state_ = reading_headers;
					return tmp;
				}
			case multipart_separator::last_chunk;
				{
					append(final);
					variant_type tmp = result_;
					result_=none;
					state_ = done;
					return tmp;
				}
			default:
				throw cppcms_error(
					(boost::format("Internal error at " __FILE__ "line %d") % __LINE__).str());
			}
			break;
		case reading_headers:
			switch(parser_.step())
			case parset::mode_data:
				return none;
			case parser::error_observerd:
				return error;
			case parser::end_of_headers:
				if(result_.which() == 0)
					return error;
				state_ = reading_data;
				return none;
			case parser::got_header:
				{
					std::string const header = parser.header_;
					parser.header_.clean();
					std::string::const_iterator m,p=header.begin();
					std::string::const_iterator e=header.end();
					p=http::protocol::skip_ws(p,e);
					m=p;
					p=http::protocol::tocken(p,e);
					std::string type(m,p); 
					if(http::protocol::compare("Content-Disposition",type)==0) 
					{
						while(p!=e) {
							if(http::protocol::separator(*p)) {
								++p;
								continue;
							}
							m=p;
							if((p=http::protocol::tocken(p,e))!=m) {
								if(http::protocol::compare(std::string(m,p),"name"))
							}
						}
									
					}
					
				}
		}
	}


private:
	multipart_separator separator_;
	cppcms::http::impl::parser parser_;
};
*/


connection::connection(cppcms::service &srv) :
	service_(&srv),
	request_in_progress_(true)
{
}

connection::~connection()
{
}


cppcms::service &connection::service()
{
	return *service_;
}
booster::shared_ptr<connection> connection::self()
{
	return shared_from_this();
}

void connection::async_prepare_request(	http::request &request,
					booster::function<void(bool)> const &h)
{
	async_read_headers(boost::bind(&connection::load_content,self(),_1,&request,h));
}

void connection::aync_wait_for_close_by_peer(booster::function<void()> const &on_eof)
{
	async_read_eof(boost::bind(&connection::handle_eof,self(),on_eof));
}

void connection::handle_eof(callback const &on_eof)
{
	if(request_in_progress_) {
		on_eof();
	}
}

void connection::set_error(ehandler const &h,std::string s)
{
	error_=s;
	h(true);
}

void connection::load_content(booster::system::error_code const &e,http::request *request,ehandler const &h)
{
	if(e)  {
		set_error(h,e.message());
		return;
	}

	std::string content_type = getenv("CONTENT_TYPE");
	std::string s_content_length=getenv("CONTENT_LENGTH");

	long long content_length = s_content_length.empty() ? 0 : atoll(s_content_length.c_str());

	if(content_length < 0)  {
		set_error(h,"Incorrect content length");
		return;
	}

	if(http::protocol::is_prefix_of("multipart/form-data",content_type)) {
		// 64 MB
		long long allowed=service().settings().get("security.multipart_form_data_limit",64*1024)*1024;
		if(content_length > allowed) { 
			set_error(h,"security violation: multipart/form-data content length too big");
			BOOSTER_NOTICE("cppcms") << "multipart/form-data size too big " << content_length << 
				" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
			return;
		}
		// FIXME
		return;
	}

	long long allowed=service().settings().get("security.content_length_limit",1024)*1024;
	if(content_length > allowed) {
		set_error(h,"security violation POST content length too big");
		BOOSTER_NOTICE("cppcms") << "POST data size too big " << content_length << 
			" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
		return;
	}

	content_.clear();

	if(content_length > 0) {
		content_.resize(content_length,0);
		async_read(	&content_.front(),
				content_.size(),
				boost::bind(&connection::on_post_data_loaded,self(),_1,request,h));
	}
	else  {
		on_post_data_loaded(booster::system::error_code(),request,h);
	}
}

void connection::on_post_data_loaded(booster::system::error_code const &e,http::request *request,ehandler const &h)
{
	if(e) { set_error(h,e.message()); return; }

	request->set_post_data(content_);

	if(!request->prepare()) {
		set_error(h,"Bad Request");
		return;
	}
	h(false);
}

bool connection::is_reuseable()
{
	return error_.empty() && keep_alive();
}

std::string connection::last_error()
{
	return error_;
}

void connection::async_write_response(	http::response &response,
					bool complete_response,
					ehandler const &h)
{
	async_chunk_=response.get_async_chunk();
	if(!async_chunk_.empty()) {
		async_write(	async_chunk_.data(),
				async_chunk_.size(),
				boost::bind(	&connection::on_async_write_written,
						self(),
						_1,
						complete_response,
						h));
		return;
	}
	if(complete_response) {
		on_async_write_written(booster::system::error_code(),complete_response,h);
		return;
	}
	service().impl().get_io_service().post(boost::bind(h,false));
}

void connection::on_async_write_written(booster::system::error_code const &e,bool complete_response,ehandler const &h)
{
	if(complete_response) {
		async_write_eof(boost::bind(&connection::on_eof_written,self(),_1,h));
		request_in_progress_=false;
		return;
	}
	service().impl().get_io_service().post(boost::bind(h,false));
}
void connection::async_complete_response(ehandler const &h)
{
	async_write_eof(boost::bind(&connection::on_eof_written,self(),_1,h));
	request_in_progress_=false;
}

void connection::on_eof_written(booster::system::error_code const &e,ehandler const &h)
{
	if(e) { set_error(h,e.message()); return; }
	h(false);
}


struct connection::reader {
	reader(connection *C,io_handler const &H,size_t S,char *P) : h(H), s(S), p(P),conn(C)
	{
		done=0;
	}
	io_handler h;
	size_t s;
	size_t done;
	char *p;
	connection *conn;
	void operator() (booster::system::error_code const &e=booster::system::error_code(),size_t read = 0)
	{
		if(e) {
			h(e,done+read);
		}
		s-=read;
		p+=read;
		done+=read;
		if(s==0)
			h(booster::system::error_code(),done);
		else
			conn->async_read_some(p,s,*this);
	}
};
struct connection::writer {
	writer(connection *C,io_handler const &H,size_t S,char const *P) : h(H), s(S), p(P),conn(C)
	{
		done=0;
	}
	io_handler h;
	size_t s;
	size_t done;
	char const *p;
	connection *conn;
	void operator() (booster::system::error_code const &e=booster::system::error_code(),size_t wr = 0)
	{
		if(e) {
			h(e,done+wr);
		}
		s-=wr;
		p+=wr;
		done+=wr;
		if(s==0)
			h(booster::system::error_code(),done);
		else
			conn->async_write_some(p,s,*this);
	}
};

void connection::async_read(void *p,size_t s,io_handler const &h)
{
	reader r(this,h,s,(char*)p);
	r();
}

void connection::async_write(void const *p,size_t s,io_handler const &h)
{
	writer w(this,h,s,(char const *)p);
	w();
}

size_t connection::write(void const *data,size_t n)
{
	char const *p=reinterpret_cast<char const *>(data);
	size_t wr=0;
	while(n > 0) {
		size_t d=write_some(p,n);
		if(d == 0)
			return wr;
		p+=d;
		wr+=d;
		n-=d;
	}
	return wr;
}

} // cgi
} // impl
} // cppcms
