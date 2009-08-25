#define CPPCMS_SOURCE
#include "asio_config.h"

#include "url_dispatcher.h"
#include "http_request.h"
#include "http_response.h"
#include "http_protocol.h"
#include "service.h"
#include "service_impl.h"
#include "json.h"
#include "cgi_api.h"
#include "util.h"

#include <boost/bind.hpp>


namespace cppcms { namespace impl { namespace cgi {

connection::connection(cppcms::service &srv) :
	service_(&srv)
{
}

connection::~connection()
{
}


cppcms::service &connection::service()
{
	return *service_;
}
intrusive_ptr<connection> connection::self()
{
	return this;
}

void connection::async_prepare_request(	http::request &request,
					boost::function<void(bool)> const &h)
{
	async_read_headers(boost::bind(&connection::load_content,self(),_1,&request,h));
}

void connection::set_error(ehandler const &h,std::string s)
{
	error_=s;
	h(true);
}

void connection::load_content(boost::system::error_code const &e,http::request *request,ehandler const &h)
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
			return;
		}
		// FIXME
		return;
	}

	long long allowed=service().settings().get("security.content_length_limit",1024)*1024;
	if(content_length > allowed) {
		set_error(h,"security violation POST content length too big");
		// TODO log
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
		on_post_data_loaded(boost::system::error_code(),request,h);
	}
}

void connection::on_post_data_loaded(boost::system::error_code const &e,http::request *request,ehandler const &h)
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
		on_async_write_written(boost::system::error_code(),complete_response,h);
		return;
	}
	service().impl().get_io_service().post(boost::bind(h,false));
}

void connection::on_async_write_written(boost::system::error_code const &e,bool complete_response,ehandler const &h)
{
	if(complete_response) {
		async_write_eof(boost::bind(&connection::on_eof_written,self(),_1,h));
		return;
	}
	service().impl().get_io_service().post(boost::bind(h,false));
}
void connection::async_complete_response(ehandler const &h)
{
	async_write_eof(boost::bind(&connection::on_eof_written,self(),_1,h));
}

void connection::on_eof_written(boost::system::error_code const &e,ehandler const &h)
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
	void operator() (boost::system::error_code const &e=boost::system::error_code(),size_t read = 0)
	{
		if(e) {
			h(e,done+read);
		}
		s-=read;
		p+=read;
		done+=read;
		if(s==0)
			h(boost::system::error_code(),done);
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
	void operator() (boost::system::error_code const &e=boost::system::error_code(),size_t wr = 0)
	{
		if(e) {
			h(e,done+wr);
		}
		s-=wr;
		p+=wr;
		done+=wr;
		if(s==0)
			h(boost::system::error_code(),done);
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
