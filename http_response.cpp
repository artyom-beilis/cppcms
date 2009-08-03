#define CPPCMS_SOURCE
#include "cgi_api.h"
#include "http_response.h"
#include "http_context.h"
#include "http_request.h"
#include "http_cookie.h"
#include "global_config.h"
#include "cppcms_error.h"
#include "service.h"
#include "config.h"
#include "util.h"

#include <iostream>
#include <sstream>
#include <iterator>
#include <map>

#include <boost/lexical_cast.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/tee.hpp>


namespace cppcms { namespace http {

namespace {

	bool string_i_comp(std::string const &left,std::string const &right)
	{
		size_t lsize=left.size();
		size_t rsize=right.size();
		for(size_t i=0;i<lsize && i<rsize;i++) {
			char cl=util::ascii_tolower(left[i]);
			char cr=util::ascii_tolower(right[i]);
			if(cl<cr) return true;
			if(cl>cr) return false;
			// if(cl==cr) continue
		}
		if(lsize<rsize)
			return true;
		return false;
	}
} // anon



namespace  {
	class output_device : public boost::iostreams::sink {
		impl::cgi::connection *conn_;
	public:
		output_device(impl::cgi::connection *conn) : conn_(conn) {}
		std::streamsize write(char const *data,std::streamsize n)
		{
			std::streamsize all=0;
			while(all < n)
				all += conn_->write_some(data,n);
			return all;
		}
	};
}

struct response::data {
	typedef bool (*compare_type)(std::string const &left,std::string const &right);
	typedef std::map<std::string,std::string,compare_type> headers_type;
	headers_type headers;
	std::vector<cookie> cookies;
	std::ostringstream buffer;
	boost::iostreams::filtering_ostream filter;
	std::ostringstream cached;
	std::ostringstream buffered;
	boost::iostreams::stream<output_device> output;

	data(impl::cgi::connection *conn) : 
		headers(string_i_comp),
		output(output_device(conn),conn->service().settings().integer("service.output_buffer_size",16384))
	{
	}
};


response::response(context &context) :
	d(new data(&context.connection())),
	context_(context),
	stream_(0),
	io_mode_(normal),
	disable_compression_(0),
	ostream_requested_(0),
	copy_to_cache_(0)
{
	set_content_header("text/html");
	if(context_.settings().integer("server.disable_xpowered_by",0)==0) {
		set_header("X-Powered-By", PACKAGE_NAME "/" PACKAGE_VERSION);
	}
}

response::~response()
{
}

void response::set_content_header(std::string const &content_type)
{
	std::string charset=context_.settings().str("l10n.charset","");
	if(charset.empty())
		set_header("Content-Type",content_type);
	else
		set_header("Content-Type",content_type+"; charset="+charset);
}
void response::set_html_header()
{
	set_content_header("text/html");
}
void response::set_xhtml_header()
{
	set_content_header("text/xhtml");
}
void response::set_plain_text_header()
{
	set_content_header("text/plain");
}
void response::set_redirect_header(std::string const &loc,int s)
{
	location(loc);
	status(s);
}
void response::set_cookie(cookie const &cookie)
{
	d->cookies.push_back(cookie);
}

void response::set_header(std::string const &name,std::string const &value)
{
	if(value.empty())
		d->headers.erase(name);
	else
		d->headers[name]=value;
}

void response::finalize()
{
	d->filter.reset();
}

std::string response::get_header(std::string const &name)
{
	data::headers_type::const_iterator p=d->headers.find(name);
	if(p!=d->headers.end())
		return p->second;
	return std::string();
}

void response::erase_header(std::string const &name)
{
	d->headers.erase(name);
}

bool response::need_gzip()
{
	if(disable_compression_)
		return false;
	if(io_mode_!=normal)
		return false;
	if(context_.settings().integer("gzip.enable",1)==0)
		return false;
	if(context_.request().http_accept_encoding().find("gzip")==std::string::npos)
		return false;
	if(!get_header("Content-Encoding").empty())
		// User had defined its own content encoding
		// he may compress data on its own... disable compression
		return false;
	std::string const content_type=get_header("Content-Type");
	// TODO fix according to RFC
	if(content_type.substr(0,5)=="text/")
		return true;
	return false;
}

response::io_mode_type response::io_mode()
{
	return io_mode_;
}

void response::io_mode(response::io_mode_type mode)
{
	if(ostream_requested_)
		throw cppcms_error("Can't set mode after requesting output stream");
	io_mode_=mode;
}

void response::write_http_headers(std::ostream &stream)
{
	std::ostream &out=d->output;
	for(data::headers_type::const_iterator h=d->headers.begin();h!=d->headers.end();++h) {
		out<<h->first<<':'<<h->second<<"\r\n";
	}
	for(unsigned i=0;i<d->cookies.size();i++) {
		out<<d->cookies[i]<<"\r\n";
	}
	out<<"\r\n";
	out<<std::flush;
}


void response::copy_to_cache(std::string const &key)
{
	copy_to_cache_=1;
	cache_key_=key;
}

std::ostream &response::out()
{
	using namespace boost::iostreams;

	if(ostream_requested_)
		return *stream_;

	std::ostream *real_sink = 0;

	if(io_mode_ == asynchronous)
		real_sink = &d->buffered;
	else
		real_sink = &d->output;

	ostream_requested_=1;
	
	write_http_headers(*real_sink);
	
	if(need_gzip()) {
		gzip_params params;

		int level=context_.settings().integer("gzip.level",-1);
		if(level!=-1)
			params.level=level;
		int buffer=context_.settings().integer("gzip.buffer",-1);
		if(buffer!=1)
			d->filter.push(gzip_compressor(params,buffer));
		else
			d->filter.push(gzip_compressor(params));

		stream_ = &d->filter;
	}
	
	if(copy_to_cache_) {
		d->filter.push(tee_filter<std::ostream>(d->cached));
		stream_ = &d->filter;
	}
	
	if(stream_)
		d->filter.push(*real_sink);
	else
		stream_=real_sink;

	return *stream_;
}

std::string response::get_async_chunk()
{
	std::string result=d->buffer.str();
	d->buffer.str("");
	return result;
}

bool response::some_output_was_written()
{
	return ostream_requested_;
}

char const *response::status_to_string(int status)
{
	switch(status) {
	case 100: return "Continue";
 	case 101: return "Switching Protocols";
 	case 200: return "OK";
 	case 201: return "Created";
 	case 202: return "Accepted";
 	case 203: return "Non-Authoritative Information";
 	case 204: return "No Content";
 	case 205: return "Reset Content";
 	case 206: return "Partial Content";
 	case 300: return "Multiple Choices";
 	case 301: return "Moved Permanently";
 	case 302: return "Found";
 	case 303: return "See Other";
 	case 304: return "Not Modified";
 	case 305: return "Use Proxy";
 	case 307: return "Temporary Redirect";
 	case 400: return "Bad Request";
 	case 401: return "Unauthorized";
 	case 402: return "Payment Required";
 	case 403: return "Forbidden";
 	case 404: return "Not Found";
 	case 405: return "Method Not Allowed";
 	case 406: return "Not Acceptable";
 	case 407: return "Proxy Authentication Required";
 	case 408: return "Request Time-out";
 	case 409: return "Conflict";
 	case 410: return "Gone";
 	case 411: return "Length Required";
 	case 412: return "Precondition Failed";
 	case 413: return "Request Entity Too Large";
 	case 414: return "Request-URI Too Large";
 	case 415: return "Unsupported Media Type";
 	case 416: return "Requested range not satisfiable";
 	case 417: return "Expectation Failed";
 	case 500: return "Internal Server Error";
 	case 501: return "Not Implemented";
 	case 502: return "Bad Gateway";
 	case 503: return "Service Unavailable";
 	case 504: return "Gateway Time-out";
 	case 505: return "HTTP Version not supported";
	default:  return "Unknown";
	}
}

void response::accept_ranges(std::string const &s) { set_header("Accept-Ranges",s); }
void response::age(unsigned seconds) { set_header("Age",boost::lexical_cast<std::string>(seconds)); }
void response::allow(std::string const &s) { set_header("Allow",s); }
void response::cache_control(std::string const &s) { set_header("Cache-Control",s); }
void response::content_encoding(std::string const &s) { set_header("Content-Encoding",s); } 
void response::content_language(std::string const &s) { set_header("Content-Language",s); }
void response::content_length(unsigned long long len)
{
	set_header("Content-Length",boost::lexical_cast<std::string>(len));
}
void response::content_location(std::string const &s) { set_header("Content-Locaton",s); }
void response::content_md5(std::string const &s) { set_header("Content-MD5",s); }
void response::content_range(std::string const &s) { set_header("Content-Range",s); }
void response::content_type(std::string const &s) { set_header("Content-Type",s); }
void response::date(time_t t) { set_header("Date",make_http_time(t)); }
void response::etag(std::string const &s) { set_header("ETag",s); }
void response::expires(time_t t) { set_header("Expires",make_http_time(t)); }
void response::last_modified(time_t t) { set_header("Last-Modified",make_http_time(t)); }
void response::location(std::string const &s) { set_header("Location",s); }
void response::pragma(std::string const &s) { set_header("Pragma",s); }
void response::proxy_authenticate(std::string const &s) { set_header("Proxy-Authenticate",s); }
void response::retry_after(unsigned n) { set_header("Retry-After",boost::lexical_cast<std::string>(n)); }
void response::retry_after(std::string const &s) { set_header("Retry-After",s); }
void response::status(int code)
{
	status(code,status_to_string(code));
}
void response::status(int code,std::string const &message)
{
	set_header("Status",boost::lexical_cast<std::string>(code)+" "+message);
}
void response::trailer(std::string const &s) { set_header("Trailer",s); }
void response::transfer_encoding(std::string const &s) { set_header("Transfer-Encoding",s); }
void response::vary(std::string const &s) { set_header("Vary",s); }
void response::via(std::string const &s) { set_header("Via",s); }
void response::warning(std::string const &s) { set_header("Warning",s); }
void response::www_authenticate(std::string const &s) { set_header("WWW-Authenticate",s); }

std::string response::make_http_time(time_t t)
{
	// RFC 2616
	// "Sun, 06 Nov 1994 08:49:37 GMT"
	std::tm tv;
	gmtime_r(&t,&tv);
	std::ostringstream ss;
	std::locale C("C");
	ss.imbue(C);
	std::time_put<char> const &put = std::use_facet<std::time_put<char> >(C);
	char const format[]="%a, %d %b %Y %H:%M:%S GMT"; 
	put.put(ss,ss,' ',&tv,format,format+sizeof(format)-1);
	return ss.str();
}


} } // http::cppcms
