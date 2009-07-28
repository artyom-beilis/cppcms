#include "cgi_api.h"

#ifdef CPPCMS_PRIV_BOOST_H
#include <cppcms_boost/bind.hpp>
namespace boost = cppcms_boost;
#else // general boost
#include <boost/bind.hpp>
#endif

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


namespace cppcms { namespace cgi { 

void connection::on_accepted()
{
	async_read_headers(boost::bind(&connection::on_headers_read,shared_from_this(),_1));
}

long long connection::check_valid_content_length()
{
	std::string content_type = getenv("CONTENT_TYPE");
	std::string s_content_length=getenv("CONTENT_LENGTH");
	long long content_length = s_content_length.empty() ? atoll(getenv("CONTENT_LENGTH")) : 0;
	
	if(content_length >0) {
		if(is_prefix_of("application/x-www-form-urlencoded",content_type)) {
			if(content_length > service().settings().ival("security.post_size_limit",4*1024)*1024) {
				return -1; 
			}
		}
		else if(is_prefix_of("multipart/form-data",content_type)) {
			long long allowed=service().settings().ival("security.post_file_size_limit",64*1024)*1024;
			if(allowed > 0 && content_length <allowed) {
				return -1; 
			}
		}
		else {
			return -1;
		}
	}
	return content_length;
}

void connection::on_headers_read(boost::system::error_code const &e)
{
	// In case of error, nothing to do, object is destroyed automatically
	if(e) return;

	long long content_length_ = check_valid_content_length();
	if(content_length_ < 0) {
		// TODO logit
		return;
	}
	loaded_content_size_ = 0;
	if(content_length_ > 0) {
		content_.resize(content_length_);
		async_read_some_post_data(&content_.front(),content_length_,
			boost::bind(&connection::on_content_recieved,shared_from_this(),
				_1,_2));
		return;
	}
	on_post_data_read();
}

void connection::on_content_recieved(boost::system::error_code const &e,size_t read)
{
	if(e || read==0) return;
	loaded_content_size_ +=read;
	if(loaded_content_size_ < content_length_) {
		async_read_some_post_data(&content_[loaded_content_size_],content_length_ - loaded_content_size_,
			boost::bind(&connection::on_content_recieved,shared_from_this(),
				_1,_2));	
	}
	else {
		on_post_data_read();
	}
}

void connection::on_post_data_read()
{
	context_.reset(new http::context(this));

	if(content_length_ > 0)
		content_.request().assign_url_encoded_post_data(&content_.front(),&content_.front()+content_length_);
	setup_application();	
}

void connection::setup_application()
{
	std::string path = getenv("PATH_INFO");
	std::string matched;

	application_ = service().applications_pool().get(path,matched);
	
	if(application_.get() == 0) {
		content_.response().status(http::response::not_found);
		content_.response().out() <<
			"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
			"	\"http://www.w3.org/TR/html4/loose.dtd\">\n"
			"<html>\n"
			"  <head>\n"
			"    <title>404 - Not Found</title>\n"
			"  </head>\n"
			"  <body>\n"
			"    <h1>404 - Not Found</h1>\n"
			"  </body>\n"
			"</html>\n";

		on_ready_response();
		return;
	}

	if(application_->dispatcher().is_async(matched)) {
		dispatch(matched,false);
	}
	else {
		service().workers_pool().post(
			boost::bind(
				&connection::dispatch,
				shared_from_this(),
				matched,
				true));
	}
}

void connection::dispatch(std::string const &path,bool in_thread)
{
	boost::function<void()> handle;
	try {
		application_->on_start();
		application_->dispatcher().run(path);
		handle=boost::bind(&connection::on_ready_response,shared_from_this());
	}
	catch(std::exception const &e){
		handle=boost::bind(&connection::on_error,shared_from_this(),std::string(e.what()));
	}
	if(in_thread)
		io_service().post(handle);
	else
		hanlde();
}

void connection::on_error(std::string const &msg)
{
	application_->on_end();
	service().applications_pool().put(application_);
	int mode = content_.response().io_mode();
	if(mode==http::response::normal || mode==http::response::nogzip) {
		content_.response().clear();
		content_.response().status(http::response::internal_server_error);
		std::ostream &out=content_.response().out();

		out<<	"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
			"	\"http://www.w3.org/TR/html4/loose.dtd\">\n"
			"<html>\n"
			"  <head>\n"
			"    <title>500 - Internal Server Error</title>\n"
			"  </head>\n"
			"  <body>\n"
			"    <h1>500 - Internal Server Error</h1>\n";
		if(service().is_developmet_mode()) {
			out<<
			"    <p>" << util::escape(msg) <<"</p>\n"; 
		}

		out<<	"  </body>\n"
			"</html>\n";
		write_response();
	}
	else {
		// Just destroy the object... Can't do more
		// TODO log the error
		return;
	}
}

void connection::on_ready_response()
{
	application_->on_end();
	service().applications_pool().put(application_);
	int mode = content_.response().io_mode();
	if(mode==http::response::normal || mode==http::response::nogzip) {
		write_response();	
	}
	else {
		// user had written its own result
		on_response_complete();
	}
}

void connection::write_response()
{
	output_written_ = 0;
	output_ = content_.response().output(output_size_);
	write_output(boost::system::error_code(),0);
}

void connection::write_output(boost::system::error_code const &e,size_t written)
{
	if(e) return; // Destroy
	output_written_ +=written;
	while(output_size_ > output_written_) {
		async_read_some_post_data(output_ + output_written_, output_size_ - output_written_,
			boost::bind(&connection::write_output,shared_from_this(),_1,_2));
	}
	on_response_complete();	
}

void connection::on_response_complete()
{
	if(keep_alive()) {
		context_.reset();
		on_accepted();
	}
}




} // cppcms::cgi
