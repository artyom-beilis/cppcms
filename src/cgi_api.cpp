///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_context.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/forwarder.h>
#include "http_protocol.h"
#include <cppcms/service.h>
#include "service_impl.h"
#include "cached_settings.h"
#include <cppcms/json.h>
#include "cgi_api.h"
#include "multipart_parser.h"
#include <cppcms/util.h>
#include <scgi_header.h>
#include <stdlib.h>
#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

#include <booster/log.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/aio_category.h>
#include <booster/aio/socket.h>
#include <booster/aio/buffer.h>


namespace cppcms { namespace impl { namespace cgi {

	//
	// Special forwarder from generic CGI to SCGI
	//
	struct connection::cgi_forwarder : public booster::enable_shared_from_this<connection::cgi_forwarder> {
	public:
		cgi_forwarder(booster::shared_ptr<connection> c,std::string ip,int port) :
			conn_(c),
			scgi_(c->get_io_service()),
			ep_(ip,port)
		{
			booster::aio::endpoint ep(ip,port);
			booster::system::error_code e;
			scgi_.open(ep.family(),e);
			if(e) {	return;	}
		}
		void async_run()
		{
			scgi_.async_connect(ep_,boost::bind(&cgi_forwarder::on_connected,shared_from_this(),_1));
		}
	private:
		void on_connected(booster::system::error_code const &e)
		{
			if(e) return;
			header_ = make_scgi_header(conn_->getenv(),0);
			scgi_.async_write(
				booster::aio::buffer(header_),
				boost::bind(&cgi_forwarder::on_header_sent,shared_from_this(),_1,_2));
		}
		void on_header_sent(booster::system::error_code const &e,size_t n)
		{
			if(e || n!=header_.size())
				return;
			header_.clear();
			std::string slen = conn_->getenv("CONTENT_LENGTH");
			content_length_ = slen.empty() ? 0LL : atoll(slen.c_str());
			if(content_length_ > 0) {
				post_.resize( content_length_ > 8192 ? 8192 : content_length_,0);
				write_post();
			}
			else {
				response_.resize(8192);
				read_response();
			}
		}
		void write_post()
		{
			if(content_length_ > 0) {
				if(content_length_ <  (long long)(post_.size())) {
					post_.resize(content_length_);
				}
				conn_->async_read_some(&post_.front(),post_.size(),
					boost::bind(&cgi_forwarder::on_post_data_read,shared_from_this(),_1,_2));
			}
			else {
				response_.swap(post_);
				response_.resize(8192);
				read_response();
			}
		}
		void on_post_data_read(booster::system::error_code const &e,size_t len)
		{
			if(e)  { cleanup(); return; }
			conn_->on_async_read_complete();
			scgi_.async_write(
				booster::aio::buffer(&post_.front(),len),
				boost::bind(&cgi_forwarder::on_post_data_written,shared_from_this(),_1,_2));
		}
		void on_post_data_written(booster::system::error_code const &e,size_t len)
		{
			if(e) { return; }
			content_length_ -= len;
			write_post();
		}
		
		void read_response() 
		{
			conn_->async_read_eof(boost::bind(&cgi_forwarder::cleanup,shared_from_this()));
			scgi_.async_read_some(booster::aio::buffer(response_),
						boost::bind(&cgi_forwarder::on_response_read,shared_from_this(),_1,_2));
		}
		void on_response_read(booster::system::error_code const &e,size_t len)
		{
			if(e) {
				conn_->async_write_eof(boost::bind(&cgi_forwarder::cleanup,shared_from_this()));
				return;
			}
			else {
				conn_->async_write(&response_.front(),len,boost::bind(&cgi_forwarder::on_response_written,shared_from_this(),_1,_2));
			}
		}
		void on_response_written(booster::system::error_code const &e,size_t /*len*/)
		{
			if(e) { cleanup(); return; }
			scgi_.async_read_some(booster::aio::buffer(response_),
				boost::bind(&cgi_forwarder::on_response_read,shared_from_this(),_1,_2));
		}

		void cleanup()
		{
			booster::system::error_code e;
			scgi_.shutdown(booster::aio::stream_socket::shut_rdwr,e);
			scgi_.close(e);
		}

		booster::shared_ptr<connection> conn_;
		booster::aio::stream_socket scgi_;
		booster::aio::endpoint ep_;
		long long int content_length_;
		std::string header_;
		std::vector<char> post_;
		std::vector<char> response_;

	};







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

void connection::async_prepare_request(	http::context *context,
					booster::callback<void(bool)> const &h)
{
	async_read_headers(boost::bind(&connection::on_headers_read,self(),_1,context,h));
}

void connection::on_headers_read(booster::system::error_code const &e,http::context *context,ehandler const &h)
{
	if(e)  {
		set_error(h,e.message());
		return;
	}
	forwarder::address_type addr = service().forwarder().check_forwading_rules(
		cgetenv("HTTP_HOST"),
		cgetenv("SCRIPT_NAME"),
		cgetenv("PATH_INFO"));
	
	if(addr.second != 0 && !addr.first.empty()) {
		booster::shared_ptr<cgi_forwarder> f(new cgi_forwarder(self(),addr.first,addr.second));
		f->async_run();
		h(true);
		return;
	}
	context->request().prepare();
	load_content(e,context,h);
}

void connection::aync_wait_for_close_by_peer(booster::callback<void()> const &on_eof)
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

void connection::load_content(booster::system::error_code const &e,http::context *context,ehandler const &h)
{
	if(e)  {
		set_error(h,e.message());
		return;
	}

	http::content_type content_type = context->request().content_type_parsed();
	char const *s_content_length=cgetenv("CONTENT_LENGTH");

	long long content_length = *s_content_length == 0 ? 0 : atoll(s_content_length);

	if(content_length < 0)  {
		set_error(h,"Incorrect content length");
		return;
	}
	
	if(content_length > 0) {
		if(content_type.media_type()=="multipart/form-data") {
			// 64 MB
			long long allowed=service().cached_settings().security.multipart_form_data_limit*1024;
			if(content_length > allowed) { 
				set_error(h,"security violation: multipart/form-data content length too big");
				BOOSTER_NOTICE("cppcms") << "multipart/form-data size too big " << content_length << 
					" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				return;
			}
			multipart_parser_.reset(new multipart_parser(
				service().cached_settings().security.uploads_path,
				service().cached_settings().security.file_in_memory_limit));
			read_size_ = content_length;
			if(!multipart_parser_->set_content_type(content_type)) {
				set_error(h,"Invalid multipart/form-data request");
				BOOSTER_NOTICE("cppcms") << "Invalid multipart/form-data request" << content_length << 
					" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				return;
			}
			content_.clear();
			content_.resize(8192);
			async_read_some(&content_.front(),content_.size(),
				boost::bind(&connection::on_some_multipart_read,
					self(),
					_1,
					_2,
					context,
					h));
		}
		else {
			long long allowed=service().cached_settings().security.content_length_limit*1024;
			if(content_length > allowed) {
				set_error(h,"security violation POST content length too big");
				BOOSTER_NOTICE("cppcms") << "POST data size too big " << content_length << 
					" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				return;
			}
			content_.clear();
			content_.resize(content_length,0);
			async_read(	&content_.front(),
					content_.size(),
					boost::bind(&connection::on_post_data_loaded,self(),_1,context,h));
		}
	}
	else  {
		on_post_data_loaded(booster::system::error_code(),context,h);
	}
}

void connection::on_some_multipart_read(booster::system::error_code const &e,size_t n,http::context *context,ehandler const &h)
{
	if(e) { set_error(h,e.message()); return; }
	read_size_-=n;
	if(read_size_ < 0) { set_error(h,"Bad request"); return ;}
	multipart_parser::parsing_result_type r = multipart_parser_->consume(&content_.front(),n);
	if(r == multipart_parser::eof) {
		if(read_size_ != 0)  {
			set_error(h,"Bad request");
			return;
		}
		content_.clear();
		multipart_parser::files_type files = multipart_parser_->get_files();
		long long allowed=service().cached_settings().security.content_length_limit*1024;
		for(unsigned i=0;i<files.size();i++) {
			if(files[i]->mime().empty() && files[i]->size() > allowed) {
				set_error(h,"Conent Lengths to big");
				return;
			}
		}
		context->request().set_post_data(files);
		multipart_parser_.reset();
		h(false);
		return;
	}
	else if (r==multipart_parser::parsing_error) {
		set_error(h,"Bad request");
		return;
	}
	else {
		async_read_some(&content_.front(),content_.size(),
			boost::bind(&connection::on_some_multipart_read,
				self(),
				_1,
				_2,
				context,
				h));
	}
}


void connection::on_post_data_loaded(booster::system::error_code const &e,http::context *context,ehandler const &h)
{
	if(e) { set_error(h,e.message()); return; }
	context->request().set_post_data(content_);
	on_async_read_complete();
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
	if(e) {	
		BOOSTER_WARNING("cppcms") << "Writing response failed:" << e.message();
		service().impl().get_io_service().post(boost::bind(h,true));
		return;
	}
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

void connection::complete_response()
{
	write_eof();
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
			return;
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
			return;
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

size_t connection::write(void const *data,size_t n,booster::system::error_code &e)
{
	char const *p=reinterpret_cast<char const *>(data);
	size_t wr=0;
	while(n > 0) {
		size_t d=write_some(p,n,e);
		if(d == 0)
			return wr;
		p+=d;
		wr+=d;
		n-=d;
		if(e)
			return wr;
	}
	return wr;
}


} // cgi
} // impl
} // cppcms
