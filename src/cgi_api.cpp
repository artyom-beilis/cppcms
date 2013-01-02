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
					ehandler const &h)
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
		h(http::context::operation_aborted);
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
	h(http::context::operation_aborted);
}

void connection::handle_http_error(int code,http::context *context,ehandler const &h)
{
	async_chunk_.clear();
	async_chunk_.reserve(256);
	std::string status;
	status.reserve(128);
	status += char('0' +  code/100);
	status += char('0' +  code/10 % 10);
	status += char('0' +  code % 10);
	status += ' ';
	status += http::response::status_to_string(code);
	if(context->service().cached_settings().service.generate_http_headers) {
		async_chunk_ += "HTTP/1.0 ";
		async_chunk_ += status;
		async_chunk_ += "\r\n"
				"Connection: close\r\n"
				"Content-Type: text/html\r\n"
				"\r\n";
	}
	else {
		async_chunk_ += "Content-Type: text/html\r\n"
				"Status: ";
		async_chunk_ += status;
		async_chunk_ += "\r\n"
				"\r\n";
	}


	async_chunk_ += 	
		"<html>\r\n"
		"<body>\r\n"
		"<h1>";
	async_chunk_ += status;
	async_chunk_ += "</h1>\r\n"
		"</body>\r\n"
		"</html>\r\n";
	async_write(async_chunk_.c_str(),async_chunk_.size(),
		boost::bind(
			&connection::handle_http_error_eof,
			self(),
			_1,
			_2,
			code,
			h));
}

void connection::handle_http_error_eof(
	booster::system::error_code const &e,
	size_t /*n*/,
	int code,
	ehandler const &h)
{
	if(e)  {
		set_error(h,e.message());
		return;
	}
	async_write_eof(boost::bind(&connection::handle_http_error_done,self(),_1,code,h));
}

void connection::handle_http_error_done(booster::system::error_code const &e,int code,ehandler const &h)
{
	if(e) {
		set_error(h,e.message());
		return;
	}
	set_error(h,http::response::status_to_string(code));
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
		handle_http_error(400,context,h);
		return;
	}
	
	if(content_length > 0) {
		if(content_type.media_type()=="multipart/form-data") {
			// 64 MB
			long long allowed=service().cached_settings().security.multipart_form_data_limit*1024;
			if(content_length > allowed) { 
				BOOSTER_NOTICE("cppcms") << "multipart/form-data size too big " << content_length << 
					" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				handle_http_error(413,context,h);
				return;
			}
			multipart_parser_.reset(new multipart_parser(
				service().cached_settings().security.uploads_path,
				service().cached_settings().security.file_in_memory_limit));
			read_size_ = content_length;
			if(!multipart_parser_->set_content_type(content_type)) {
				BOOSTER_NOTICE("cppcms") << "Invalid multipart/form-data request" << content_length << 
					" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				handle_http_error(400,context,h);
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
				BOOSTER_NOTICE("cppcms") << "POST data size too big " << content_length << 
					" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				handle_http_error(413,context,h);
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
	if(read_size_ < 0) { handle_http_error(400,context,h); return ;}
	multipart_parser::parsing_result_type r = multipart_parser_->consume(&content_.front(),n);
	if(r == multipart_parser::eof) {
		if(read_size_ != 0)  {
			handle_http_error(400,context,h);
			return;
		}
		content_.clear();
		multipart_parser::files_type files = multipart_parser_->get_files();
		long long allowed=service().cached_settings().security.content_length_limit*1024;
		for(unsigned i=0;i<files.size();i++) {
			if(files[i]->mime().empty() && files[i]->size() > allowed) {
				BOOSTER_NOTICE("cppcms") << "multipart/form-data non-file entry size too big " << 
						files[i]->size() 
						<< " REMOTE_ADDR = `" << getenv("REMOTE_ADDR") 
						<< "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				handle_http_error(413,context,h);
				return;
			}
		}
		context->request().set_post_data(files);
		multipart_parser_.reset();
		h(http::context::operation_completed);
		return;
	}
	else if (r==multipart_parser::parsing_error) {
		handle_http_error(400,context,h);
		return;
	}
	else if(r==multipart_parser::no_room_left) {
		handle_http_error(413,context,h);
		return;
	}
	else if(read_size_ == 0) {
		handle_http_error(400,context,h);
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
	h(http::context::operation_completed);
}

bool connection::is_reuseable()
{
	return error_.empty() && keep_alive();
}

std::string connection::last_error()
{
	return error_;
}

struct connection::async_write_binder : public booster::callable<void(booster::system::error_code const &,size_t)> {
	typedef booster::shared_ptr<cppcms::impl::cgi::connection> self_type;
	self_type self;
	ehandler h;
	bool complete_response;
	booster::shared_ptr<std::vector<char> > block;
	async_write_binder() :complete_response(false) {}
	void init(self_type c,bool comp,ehandler const &hnd,booster::shared_ptr<std::vector<char> > const &b) 
	{
		self=c;
		complete_response = comp;
		h = hnd;
		block = b;
	}
	void reset()
	{
		h=ehandler();
		self.reset();
		complete_response = false;
		block.reset();
	}
	void operator()(booster::system::error_code const &e,size_t)
	{
		self->on_async_write_written(e,complete_response,h);
		if(!self->cached_async_write_binder_) {
			self->cached_async_write_binder_ = this;
			reset();
		}
	}
};


void connection::async_write_response(	http::response &response,
					bool complete_response,
					ehandler const &h)
{
	http::response::chunk_type chunk = response.get_async_chunk();
	if(chunk.second > 0) {
		booster::intrusive_ptr<async_write_binder> binder;
		binder.swap(cached_async_write_binder_);
		if(!binder)
			binder = new async_write_binder();

		binder->init(self(),complete_response,h,chunk.first);
		booster::intrusive_ptr<booster::callable<void(booster::system::error_code const &,size_t)> > p(binder.get());
		async_write(	&(*chunk.first)[0],
				chunk.second,
				p);
		return;
	}
	if(!complete_response) {
		// request to send an empty block
		service().impl().get_io_service().post(boost::bind(h,http::context::operation_completed));
		return;
	}
	on_async_write_written(booster::system::error_code(),true,h); // < h will not be called when complete_response = true
}

void connection::on_async_write_written(booster::system::error_code const &e,bool complete_response,ehandler const &h)
{
	if(e) {	
		BOOSTER_WARNING("cppcms") << "Writing response failed:" << e.message();
		service().impl().get_io_service().post(boost::bind(h,http::context::operation_aborted));
		return;
	}
	if(complete_response) {
		async_write_eof(boost::bind(&connection::on_eof_written,self(),_1,h));
		request_in_progress_=false;
		return;
	}
	h(http::context::operation_completed);
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
	h(http::context::operation_completed);
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

void connection::async_read(void *p,size_t s,io_handler const &h)
{
	reader r(this,h,s,(char*)p);
	r();
}

} // cgi
} // impl
} // cppcms
