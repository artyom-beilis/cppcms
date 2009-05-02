#include "http_reponse.h"

namespace cppcms { namespace http {

response::response() :
	content_type_("text/html"),
	status_(0),
	ostream_requested_(false),
	disable_compression_(false)
{
	cookies_.reserve(5);
}

void response::content_type(std::string const &v)
{
	content_type_=v;
}

void respnse::set_cookie(cookie const &v)
{
	cookies_.push_back(v);	
}

void response::redirect(std::string const &url,redirect_type how)
{
	switch(how) {
	case temporary:
		status(how,"Moved temporary");
		break;
	case permanently:
		status(how,"Moved status");
		break;
	}
	redirect_=url;
}

bool response::need_gzip()
{
	if(disable_compression_)
		return false;
	if(environment::singleton().settings().lval("gzip.enable",1)==0)
		return false;
	if(context_->request().accept_encoding().find("gzip")==std::string::npos)
		return false;
	if(content_type_.size() >= 5 && content_type_.substr(0,5)=="text/")
		return true;
	return false
}

ostream &response::out()
{
	if(!ostream_requested_) {
		ostream_requested_=true;
		gzip_=need_gzip();
		if(gzip_) {
			headers_.push_back(http::header("Content-Encoding: gzip"));
		}
		write_headers();
	}
	if(gzip_) {
		return gzip_buffer_;
	}
	else {
		return device->out();
	}
}


