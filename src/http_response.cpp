///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "cgi_api.h"
#include <cppcms/http_response.h>
#include <cppcms/http_context.h>
#include <cppcms/http_request.h>
#include <cppcms/http_cookie.h>
#include "http_protocol.h"
#include "cached_settings.h"
#include <cppcms/json.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/service.h>
#include <cppcms/config.h>
#include <cppcms/localization.h>
#include <cppcms/util.h>
#include <cppcms/session_interface.h>

#include <booster/log.h>
#include <booster/posix_time.h>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <iterator>
#include <map>
#include <list>
#include <stdio.h>

#ifndef CPPCMS_NO_GZIP
#include <zlib.h>
#endif

//#include <booster/streambuf.h>

namespace cppcms { namespace http {

namespace details {

	bool string_i_comp(std::string const &left,std::string const &right)
	{
		return http::protocol::compare(left,right) < 0;
	}
	template<typename T>
	std::string itoa(T v)
	{
		std::ostringstream ss;
		ss.imbue(std::locale::classic());
		ss<<v;
		return ss.str();
	}

	class extended_streambuf : public std::streambuf {
	public:
		virtual void close() = 0;
		virtual ~extended_streambuf() {}
	};


	class copy_buf : public extended_streambuf  {
	public:
		copy_buf(std::streambuf *output = 0) :
			out_(output)
		{
		}
		void open(std::streambuf *out) 
		{
			out_ = out;
		}
		size_t getstr(booster::shared_ptr<std::vector<char> > &buf)
		{
			size_t n = buffer_.size() -  (epptr() - pptr());
			setp(0,0);
			if(!borrowed_buffer_) {
				borrowed_buffer_.reset(new std::vector<char>());
			}
			borrowed_buffer_->swap(buffer_);
			buf = borrowed_buffer_;
			return n;
		}
		void getstr(std::string &out)
		{
			size_t n = buffer_.size() -  (epptr() - pptr());
			setp(0,0);
			if(n!=0) {
				out.assign(&buffer_[0],n);
			}
			else {
				out.clear();
			}
		}
		int overflow(int c)
		{
			// we relay on a fact that std::string is continuous
			// storage so we can eliminate some memory copying
			int r=0;
			if(out_ && pbase()!=pptr()) {
				int n = pptr()-pbase();
				if(out_->sputn(pbase(),n)!=n)
					r=-1;
			}
			if(pptr() == 0) {
				if(buffer_.empty()) {
					if(borrowed_buffer_ && borrowed_buffer_.unique() && borrowed_buffer_->size()!=0) {
						buffer_.swap(*borrowed_buffer_);
					}
					else {
						buffer_.resize(128);
					}
				}
				setp(&buffer_[0],&buffer_[0]+buffer_.size());
			}
			else if(pptr()==epptr()) {
				size_t size = buffer_.size();
				buffer_.resize(size * 2);
				setp(&buffer_[size],&buffer_[size]+size);
			}
			else {
				setp(pptr(),epptr());
			}
			if(r==0 && c!=EOF)
				sputc(c);
			return r;
		}
		int sync()
		{
			if(overflow(EOF) < 0)
				return -1;
			if(out_) 
				return out_->pubsync();
			return 0;
		}
		void close()
		{
			overflow(EOF);
			out_ = 0;
		}
	private:
		booster::shared_ptr<std::vector<char> > borrowed_buffer_;
		std::vector<char> buffer_;
		std::streambuf *out_;
	};




#ifndef CPPCMS_NO_GZIP
	class gzip_buf : public extended_streambuf {
	public:
		gzip_buf() :
			opened_(false),
			z_stream_(z_stream()),
			out_(0)
		{
		}
		int overflow(int c)
		{
			if(pbase()==epptr())
				return -1;
			int have = pptr() - pbase();
			if(have > 0) {
				if(do_write(pbase(),have,Z_NO_FLUSH) < 0)
					return -1;
				pbump(-have);
			}
			if(c!=EOF) {
				sputc(c);
			}
			return 0;
		}
		int sync()
		{
			int have = pptr() - pbase();
			if(do_write(pbase(),have,Z_SYNC_FLUSH) < 0)
				return -1;
			pbump(-have);
			return 0;
		}
		int do_write(char const *p,int n,int flush_flag)
		{
			if(!out_ || !opened_) {
				return -1;
			}

			if(n==0 && flush_flag==Z_NO_FLUSH) {
				return 0;
			}

			z_stream_.avail_in = n;
			z_stream_.next_in = (Bytef*)(p);

			do {
				z_stream_.avail_out = buffer_;
				z_stream_.next_out = (Bytef*)(&chunk_[0]);
				deflate(&z_stream_,flush_flag);
				int have = chunk_.size() - z_stream_.avail_out;
				if(out_->sputn(&chunk_[0],have)!=have) {
					out_ = 0;
					return -1;
				}
			} while(z_stream_.avail_out == 0);

			if(flush_flag == Z_SYNC_FLUSH && out_ && out_->pubsync() < 0) {
				out_ = 0;
				return -1;
			}

			return 0;
		}
		void close()
		{
			if(!opened_)
				return;
			do_write(pbase(),pptr()-pbase(),Z_FINISH);
			deflateEnd(&z_stream_);
			opened_ = false;
			z_stream_ = z_stream();
			chunk_.clear();
			in_chunk_.clear();
			out_ = 0;

		}
		void open(extended_streambuf *out,int level,int buffer_size)
		{
			level_ = level;
			if(buffer_size < 256)
				buffer_size = 256;
			buffer_ = buffer_size;
			out_ = out;
			if(deflateInit2(&z_stream_,
					level_,
					Z_DEFLATED,
					15 + 16, // 15 window bits+gzip = 16,
					8, // memuse
					Z_DEFAULT_STRATEGY) != Z_OK)
			{
				std::string error = "ZLib init failed";
				if(z_stream_.msg) {
					error+=":";
					error+=z_stream_.msg;
				}
				throw booster::runtime_error(error);
			}
			in_chunk_.resize(buffer_);
			chunk_.resize(buffer_);
			setp(&in_chunk_[0],&in_chunk_[0]+buffer_);
			opened_ = true;
		}
		~gzip_buf()
		{
			if(opened_)
				deflateEnd(&z_stream_);
		}
	private:
		bool opened_;
		std::vector<char> chunk_;
		std::vector<char> in_chunk_;
		z_stream z_stream_;
		extended_streambuf *out_;
		int level_;
		size_t buffer_;
	};

#endif
	class basic_device : public extended_streambuf {
	public:
		basic_device() : 
			final_(false),
			eof_send_(false),
			buffer_size_(0)
		{
		}
		void open(booster::weak_ptr<impl::cgi::connection> c,size_t n)
		{
			buffer_size_ = n;
			do_setp();
			conn_ = c;
		}

		virtual bool do_write(impl::cgi::connection &c,booster::aio::const_buffer const &out,bool eof,booster::system::error_code &e) = 0;

		int write(booster::aio::const_buffer const &out)
		{
			booster::system::error_code e;
			return write(out,e);
		}
		
		int write(booster::aio::const_buffer const &out,booster::system::error_code &e)
		{
			bool send_eof = final_ && !eof_send_;
			if(out.empty() && !send_eof) 
				return 0;
			booster::shared_ptr<impl::cgi::connection> c = conn_.lock();
			if(!c)
				return -1;
			eof_send_ = send_eof;
			if(do_write(*c,out,send_eof,e)) {
				return 0;
			}
			if(e) {
				BOOSTER_WARNING("cppcms") << "Failed to write response:" << e.message();
				conn_.reset();
				return -1;
			}
			return 0;
		}
		virtual int sync()
		{
			return overflow(EOF);
		}
		virtual int overflow(int c)
		{
			char c_tmp=c;
			booster::aio::const_buffer out=booster::aio::buffer(pbase(),pptr()-pbase());
			if(c!=EOF)
				out += booster::aio::buffer(&c_tmp,1);
			booster::system::error_code e;
			if(write(out)!=0)
				return -1;
			do_setp();
			return 0;
		}
		virtual std::streamsize xsputn(const char* s, std::streamsize n)
		{
			if((epptr() - pptr()) >= n) {
				memcpy(pptr(),s,n);
				pbump(n);
			}
			else {
				booster::aio::const_buffer out=booster::aio::buffer(pbase(),pptr()-pbase());
				out+=booster::aio::buffer(s,n);
				if(write(out)!=0)
					return -1;
				do_setp();
			}
			return n;
		}

		void do_setp()
		{
			output_.resize(buffer_size_);
			if(buffer_size_ == 0)
				setp(0,0);
			else
				setp(&output_[0],&output_[buffer_size_-1]+1);
		}
		int flush(booster::system::error_code &e)
		{
			int r = write(booster::aio::buffer(pbase(),pptr()-pbase()),e);
			setp(pbase(),epptr());
			return r;
		}
		virtual std::streambuf *setbuf(char *,std::streamsize size)
		{
			buffer_size_ = size;
			std::streamsize content_size = pptr() - pbase();
			if(content_size > size) {
				booster::system::error_code e;
				if(flush(e)!=0)
					return 0;
				content_size = 0;
			}
			do_setp();
			pbump(content_size);
			return this;
		}
		void close()
		{
			if(eof_send_)
				return;
			final_=true;
			booster::system::error_code e;
			flush(e);
		}
	protected:
		booster::weak_ptr<impl::cgi::connection> conn_;
		bool final_;
		bool eof_send_;
		size_t buffer_size_;
		std::vector<char> output_;
	};

	class async_io_buf : public basic_device {
	public:
		async_io_buf() : full_buffering_(true)
		{
		}

		int full_buffering(bool buffering)
		{
			if(full_buffering_ == buffering)
				return 0;
			full_buffering_ = buffering;
			if(full_buffering_ == false) {
				if(pubsetbuf(0,buffer_size_)==0) {
					return -1;
				}
			}
			return 0;
		}
		bool full_buffering()
		{
			return full_buffering_;
		}
		std::streambuf *setbuf(char *s,std::streamsize size)
		{
			if(full_buffering_) {
				buffer_size_ = size;
				std::streamsize content_size = pptr() - pbase();
				if(size_t(size) > output_.size())
					output_.resize(size);
				do_setp();
				pbump(content_size);
				return this;
			}
			return basic_device::setbuf(s,size);
		}
		size_t next_size(size_t in)
		{
			if(in == 0)
				return 64;
			return in * 2;
			
		}
		virtual int overflow(int c)
		{
			if(full_buffering_) {
				if(pptr() ==  epptr()) {
					size_t current_size = pptr() - pbase();
					output_.resize(next_size(output_.size()));
					setp(&output_[0],&output_[output_.size()-1]+1);
					pbump(current_size);
				}
				if(c!=EOF) {
					*pptr() = c;
					pbump(1);
				}
				return 0;
			}
			else {
				return basic_device::overflow(c);
			}
		}
		virtual std::streamsize xsputn(const char* s, std::streamsize n)
		{
			if(full_buffering_) {
				std::streamsize reminder = epptr() - pptr();
				if(reminder < n) {
					size_t current_size = pptr()-pbase();
					size_t minimal_size = current_size + n;
					size_t resize_size = next_size(output_.size());
					while(resize_size < minimal_size)
						resize_size *= 2;
					output_.resize(resize_size);
					setp(&output_[0],&output_[0]+resize_size);
					pbump(current_size);
				}
				memcpy(pptr(),s,n);
				pbump(n);
				return n;
			}
			else {
				return basic_device::xsputn(s,n);
			}
		}
		virtual bool do_write(impl::cgi::connection &c,booster::aio::const_buffer const &out,bool eof,booster::system::error_code &e) 
		{
			c.nonblocking_write(out,eof,e);
			if(e)
				return false;
			return true;
		}
	private:
		bool full_buffering_;
	};


	class output_device : public basic_device {
	public:
		virtual bool do_write(impl::cgi::connection &c,booster::aio::const_buffer const &out,bool eof,booster::system::error_code &e) 
		{
			return c.write(out,eof,e);
		}
	};


}

struct response::_data {
	typedef bool (*compare_type)(std::string const &left,std::string const &right);
	typedef std::map<std::string,std::string,compare_type> headers_type;
	headers_type headers;
	std::list<std::string> added_headers;
	std::list<details::extended_streambuf *> buffers;

	details::async_io_buf buffered;
	details::copy_buf cached;
	#ifndef CPPCMS_NO_GZIP
	details::gzip_buf zbuf;
	#endif
	details::output_device output_buf;
	std::ostream output;
	booster::weak_ptr<impl::cgi::connection> conn;
	int required_buffer_size;
	bool full_buffering;

	_data(impl::cgi::connection *c) : 
		headers(details::string_i_comp),
		output(0),
		conn(c->shared_from_this()),
		required_buffer_size(-1),
		full_buffering(true)

	{
	}
};


using details::itoa;

response::response(context &context) :
	d(new _data(&context.connection())),
	context_(context),
	stream_(0),
	io_mode_(asynchronous),
	disable_compression_(0),
	ostream_requested_(0),
	copy_to_cache_(0),
	finalized_(0)
{
	set_content_header("text/html");
	if(context_.service().cached_settings().service.disable_xpowered_by==false) {
		if(context_.service().cached_settings().service.disable_xpowered_by_version)
			set_header("X-Powered-By", CPPCMS_PACKAGE_NAME);
		else
			set_header("X-Powered-By", CPPCMS_PACKAGE_NAME "/" CPPCMS_PACKAGE_VERSION);
	}
}

response::~response()
{
}

void response::set_content_header(std::string const &content_type)
{
	if(context_.service().cached_settings().localization.disable_charset_in_content_type) {
		set_header("Content-Type",content_type);
	}
	else {
		std::string charset=std::use_facet<locale::info>(context_.locale()).encoding();
		set_header("Content-Type",content_type+"; charset="+charset);
	}
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
	std::ostringstream ss;
	ss << cookie;
	d->added_headers.push_back(ss.str());
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
	if(!finalized_) {
		out();
		for(std::list<details::extended_streambuf *>::iterator p=d->buffers.begin();p!=d->buffers.end();++p)
			(*p)->close();
		finalized_=1;
	}
}

void response::setbuf(int buffer_size)
{
	if(buffer_size < 0)
		buffer_size = -1;
	d->required_buffer_size = buffer_size;

	if(ostream_requested_) {
		if(buffer_size < 0) {
			if(io_mode_ == asynchronous || io_mode_ == asynchronous_raw)  
				buffer_size = context_.service().cached_settings().service.async_output_buffer_size;
			else
				buffer_size = context_.service().cached_settings().service.output_buffer_size;
		}
		d->buffers.back()->pubsetbuf(0,buffer_size);
	}
}

void response::full_asynchronous_buffering(bool enable)
{
	d->buffered.full_buffering(enable);
}
bool response::full_asynchronous_buffering()
{
	return d->buffered.full_buffering();
}
bool response::pending_blocked_output()
{
	booster::shared_ptr<impl::cgi::connection> conn = d->conn.lock();
	if(!conn)
		return false;
	return conn->has_pending();
}


std::string response::get_header(std::string const &name)
{
	_data::headers_type::const_iterator p=d->headers.find(name);
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
#ifndef CPPCMS_NO_GZIP
	if(disable_compression_)
		return false;
	if(io_mode_!=normal)
		return false;
	if(context_.service().cached_settings().gzip.enable==false)
		return false;
	if(strstr(context_.request().cgetenv("HTTP_ACCEPT_ENCODING"),"gzip")==0)
		return false;
	if(!get_header("Content-Encoding").empty())
		// User had defined its own content encoding
		// he may compress data on its own... disable compression
		return false;
	std::string const content_type=get_header("Content-Type");
	if(protocol::is_prefix_of("text/",content_type))
		return true;
	return false;
#else
	return false;
#endif
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

void response::write_http_headers(std::ostream &out)
{
	context_.session().save();
	
	_data::headers_type::const_iterator p = d->headers.end();

	if(context_.service().cached_settings().service.generate_http_headers) {
		p=d->headers.find("Status");
		if(p == d->headers.end())
			out << "HTTP/1.0 200 Ok\r\n";
		else
			out << "HTTP/1.0 " << p->second <<"\r\n";
	}
	
	for(_data::headers_type::const_iterator h=d->headers.begin();h!=d->headers.end();++h) {
		if(h==p)
			continue;
		out<<h->first<<": "<<h->second<<"\r\n";
	}

	for(std::list<std::string>::const_iterator p=d->added_headers.begin();p!=d->added_headers.end();++p) {
		out << *p << "\r\n";
	}

	out<<"\r\n";
	out<<std::flush;
}
void response::add_header(std::string const &name,std::string const &value)
{
	std::string h;
	h.reserve(name.size() + value.size() + 3);
	h+=name;
	h+=": ";
	h+=value;
	d->added_headers.push_back(std::string());
	d->added_headers.back().swap(h);
}


void response::copy_to_cache()
{
	copy_to_cache_=1;
}

std::string response::copied_data()
{
	std::string tmp;
	if(!copy_to_cache_ || !ostream_requested_)
		return tmp;
	d->cached.getstr(tmp);
	return tmp;
}

int response::flush_async_chunk(booster::system::error_code &e)
{
	return d->buffered.flush(e);
}


std::ostream &response::out()
{
	if(ostream_requested_)
		return d->output;

	if(finalized_)
		throw cppcms_error("Request for output stream for finalized request is illegal");
	
	if(io_mode_ == asynchronous || io_mode_ == asynchronous_raw)  {
		size_t bsize = context_.service().cached_settings().service.async_output_buffer_size;
		if(d->required_buffer_size != -1)
			bsize = d->required_buffer_size;
		d->buffered.open(d->conn,bsize);
		d->output.rdbuf(&d->buffered);
		d->buffers.push_front(&d->buffered);
	}
	else { 
		size_t bsize = context_.service().cached_settings().service.output_buffer_size;
		if(d->required_buffer_size != -1)
			bsize = d->required_buffer_size;
		d->output_buf.open(d->conn,bsize);
		d->output.rdbuf(&d->output_buf);
		d->buffers.push_front(&d->output_buf);
	}
	
	ostream_requested_=1;
	
	#ifndef CPPCMS_NO_GZIP
	bool gzip = need_gzip();
	
	if(gzip) {
		content_encoding("gzip");
	}
	#endif

	// Now we shoulde write headers -- before comrpession
	if(io_mode_ != raw && io_mode_ != asynchronous_raw)
		write_http_headers(d->output);
	
	if(copy_to_cache_) {
		d->cached.open(d->output.rdbuf());
		d->output.rdbuf(&d->cached);
		d->buffers.push_front(&d->cached);
	}
	
	#ifndef CPPCMS_NO_GZIP
	if(gzip) {
		int level=context_.service().cached_settings().gzip.level;
		int buffer=context_.service().cached_settings().gzip.buffer;
		d->zbuf.open(d->buffers.front(),level,buffer);
		d->output.rdbuf(&d->zbuf);
		d->buffers.push_front(&d->zbuf);
	}
	#endif
	
	d->output.imbue(context_.locale());
	return d->output;
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

void response::make_error_response(int stat,std::string const &msg)
{
	status(stat);
	out() <<"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
		"	\"http://www.w3.org/TR/html4/loose.dtd\">\n"
		"<html>\n"
		"  <head>\n"
		"    <title>"<<stat<<" &mdash; "<< http::response::status_to_string(stat)<<"</title>\n"
		"  </head>\n"
		"  <body>\n"
		"    <h1>"<<stat<<" &mdash; "<< http::response::status_to_string(stat)<<"</h1>\n";
	if(!msg.empty()) {
		out()<<"    <p>"<<util::escape(msg)<<"</p>\n";
	}
	out()<<	"  </body>\n"
		"</html>\n"<<std::flush;
}


void response::accept_ranges(std::string const &s) { set_header("Accept-Ranges",s); }
void response::age(unsigned seconds) { set_header("Age",itoa(seconds)); }
void response::allow(std::string const &s) { set_header("Allow",s); }
void response::cache_control(std::string const &s) { set_header("Cache-Control",s); }
void response::content_encoding(std::string const &s) { set_header("Content-Encoding",s); } 
void response::content_language(std::string const &s) { set_header("Content-Language",s); }
void response::content_length(unsigned long long len)
{
	set_header("Content-Length",itoa(len));
}
void response::content_location(std::string const &s) { set_header("Content-Location",s); }
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
void response::retry_after(unsigned n) { set_header("Retry-After",itoa(n)); }
void response::retry_after(std::string const &s) { set_header("Retry-After",s); }
void response::status(int code)
{
	status(code,status_to_string(code));
}
void response::status(int code,std::string const &message)
{
	set_header("Status",itoa(code)+" "+message);
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

	std::tm tv=booster::ptime::universal_time(booster::ptime(t));

	std::ostringstream ss;
	std::locale C=std::locale::classic();
	ss.imbue(C);

	std::time_put<char> const &put = std::use_facet<std::time_put<char> >(C);
	char const format[]="%a, %d %b %Y %H:%M:%S GMT"; 
	put.put(ss,ss,' ',&tv,format,format+sizeof(format)-1);
	return ss.str();
}


} } // http::cppcms
