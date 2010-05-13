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
//#define DEBUG_HTTP_PARSER
#include "cgi_api.h"
#include "cgi_acceptor.h"
#include "service.h"
#include "service_impl.h"
#include "cppcms_error_category.h"
#include "json.h"
#include "http_parser.h"
#include "config.h"
#include "util.h"
#include <string.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

#include <booster/aio/buffer.h>

namespace io = booster::aio;


namespace cppcms {
namespace impl {
namespace cgi {
	class http : public connection {
	public:
		http(cppcms::service &srv) :
			connection(srv),
			socket_(srv.impl().get_io_service()),
			input_body_ptr_(0),
			input_parser_(input_body_,input_body_ptr_),
			output_body_ptr_(0),
			output_parser_(output_body_,output_body_ptr_),
			headers_done_(false),
			first_header_observerd_(false),
			total_read_(0)
		{

			env_["SERVER_SOFTWARE"]=PACKAGE_NAME "/" PACKAGE_VERSION;
			env_["SERVER_NAME"]=srv.settings().get("service.ip","127.0.0.1");
			std::ostringstream ss;
			ss.imbue(std::locale::classic());
			ss << srv.settings().get("service.port",8080);
			env_["SERVER_PORT"]=ss.str();
			env_["GATEWAY_INTERFACE"]="CGI/1.0";
			env_["SERVER_PROTOCOL"]="HTTP/1.0";

		}
		~http()
		{
			booster::system::error_code e;
			socket_.shutdown(io::socket::shut_rdwr,e);
		}
		struct binder {
			void operator()(booster::system::error_code const &e,size_t n) const
			{
				self_->some_headers_data_read(e,n,h_);
			}
			binder(booster::shared_ptr<http> self,handler const &h) :
				self_(self),
				h_(h)
			{
			}
		private:
			booster::shared_ptr<http> self_;
			handler h_;
		};
		virtual void async_read_headers(handler const &h)
		{
			input_body_.reserve(8192);
			input_body_.resize(8192,0);
			input_body_ptr_=0;
			socket_.async_read_some(io::buffer(input_body_),binder(self(),h));
		}

		void some_headers_data_read(booster::system::error_code const &e,size_t n,handler const &h)
		{
			if(e) { h(e); return; }

			total_read_+=n;
			if(total_read_ > 16384) {
				h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}

			input_body_.resize(n);

			for(;;) {
				using ::cppcms::http::impl::parser;
				switch(input_parser_.step()) {
				case parser::more_data:
					// Assuming body_ptr == body.size()
					async_read_headers(h);
					return;
				case parser::got_header:
					if(!first_header_observerd_) {
						first_header_observerd_=true;
						std::string::iterator rmethod,query=input_parser_.header_.end();
						rmethod=std::find(	input_parser_.header_.begin(),
									input_parser_.header_.end(),
									' ');
						if(rmethod!=input_parser_.header_.end()) 
							query=std::find(rmethod+1,input_parser_.header_.end(),' ');
						if(query!=input_parser_.header_.end()) {
							request_method_.assign(input_parser_.header_.begin(),rmethod);
							request_uri_.assign(rmethod+1,query);
							first_header_observerd_=true;
						}
						else {
							h(booster::system::error_code(errc::protocol_violation,cppcms_category));
							return;
						}
					}
					else { // Any other header
						std::string name;
						std::string value;
						if(!parse_single_header(input_parser_.header_,name,value))  {
							h(booster::system::error_code(errc::protocol_violation,cppcms_category));
							return;
						}
						if(name=="CONTENT_LENGTH" || name=="CONTENT_TYPE")
							env_[name]=value;
						else
							env_["HTTP_"+name]=value;
					}
					break;
				case parser::end_of_headers:
					process_request(h);				
					return;
				case parser::error_observerd:
					h(booster::system::error_code(errc::protocol_violation,cppcms_category));
					return;
				}
			}


		}

		// should be called only after headers are read
		virtual std::string getenv(std::string const &key)
		{
			std::map<std::string,std::string>::const_iterator p;
			p=env_.find(key);
			if(p==env_.end())
				return std::string();
			return p->second;
		}
		virtual std::map<std::string,std::string> const &getenv()
		{
			return env_;
		}
		virtual void async_read_some(void *p,size_t s,io_handler const &h)
		{
			if(input_body_ptr_==input_body_.size()) {
				input_body_.clear();
				input_body_ptr_=0;
			}
			if(!input_body_.empty()) {
				if(input_body_.size() - input_body_ptr_ < s) {
					s=input_body_.size() -  input_body_ptr_;
				}
				memcpy(p,&input_body_[input_body_ptr_],s);
				input_body_ptr_+=s;
				if(input_body_ptr_==input_body_.size()) {
					input_body_.clear();
					input_body_ptr_=0;
				}
				socket_.get_io_service().post(boost::bind(h,booster::system::error_code(),s));
				return;
			}
			socket_.async_read_some(io::buffer(p,s),h);
		}
		virtual void async_write_eof(handler const &h)
		{
			booster::system::error_code e;
			socket_.shutdown(io::socket::shut_wr,e);
			socket_.get_io_service().post(boost::bind(h,booster::system::error_code()));
		}
		virtual void async_write_some(void const *p,size_t s,io_handler const &h)
		{
			if(headers_done_)
				socket_.async_write_some(io::buffer(p,s),h);
			else
				process_output_headers(p,s,h);
		}
		virtual size_t write_some(void const *buffer,size_t n)
		{
			if(headers_done_)
				return socket_.write_some(io::buffer(buffer,n));
			return process_output_headers(buffer,n);
		}
		virtual booster::aio::io_service &get_io_service()
		{
			return socket_.get_io_service();
		}
		virtual bool keep_alive()
		{
			return false;
		}

		virtual void close()
		{
			booster::system::error_code e;
			socket_.shutdown(io::socket::shut_rd,e);
			socket_.close(e);
		}
		virtual void async_read_eof(callback const &h)
		{
			static char a;
			socket_.async_read_some(io::buffer(&a,1),boost::bind(h));
		}

	private:
		size_t process_output_headers(void const *p,size_t s,io_handler const &h=io_handler())
		{
			char const *ptr=reinterpret_cast<char const *>(p);
			output_body_.insert(output_body_.end(),ptr,ptr+s);

			using cppcms::http::impl::parser;

			for(;;) {
				switch(output_parser_.step()) {
				case parser::more_data:
					if(!h.empty())
						h(booster::system::error_code(),s);
					return s;
				case parser::got_header:
					{
						std::string name,value;
						if(!parse_single_header(output_parser_.header_,name,value))  {
							h(booster::system::error_code(errc::protocol_violation,cppcms_category),s);
							return s;
						}
						if(name=="STATUS") {
							response_line_ = "HTTP/1.0 "+value+"\r\n";

							return write_response(h,s);
						}
					}
					break;
				case parser::end_of_headers:
					response_line_ = "HTTP/1.0 200 Ok\r\n";

					return write_response(h,s);
				case parser::error_observerd:
					h(booster::system::error_code(errc::protocol_violation,cppcms_category),0);
					return 0;
				}
			}
	
		}
		size_t write_response(io_handler const &h,size_t s)
		{
			char const *addon = 
				"Server: CppCMS-Embedded/" PACKAGE_VERSION "\r\n"
				"Connection: close\r\n";

			response_line_.append(addon);

			booster::aio::const_buffer packet = 
				io::buffer(response_line_) 
				+ io::buffer(output_body_);
#ifdef DEBUG_HTTP_PARSER
			std::cerr<<"["<<response_line_<<std::string(output_body_.begin(),output_body_.end())
				<<"]"<<std::endl;
#endif
			headers_done_=true;
			if(h.empty()) {
				booster::system::error_code e;
				socket_.write(packet,e);
				if(e) return 0;
				return s;
			}

			socket_.async_write(packet,boost::bind(&http::do_write,self(),_1,h,s));
			return s;
		}

		void do_write(booster::system::error_code const &e,io_handler const &h,size_t s)
		{
			if(e) { h(e,0); return; }
			h(booster::system::error_code(),s);
		}

		void process_request(handler const &h)
		{
			if(request_method_!="GET" && request_method_!="POST" && request_method_!="HEAD") {
				response("HTTP/1.0 501 Not Implemented\r\n\r\n",h);
				return;
			}

			env_["REQUEST_METHOD"]=request_method_;
			std::string remote_addr;
			if(service().settings().get("http.proxy.behind",false)) {
				remote_addr=socket_.remote_endpoint().ip();
			}
			else {
				std::vector<std::string> default_headers;
				default_headers.push_back("X-Forwarded-For");

				std::vector<std::string> headers = 
					service().settings().get("http.proxy.remote_addr_headers",default_headers);

				for(unsigned i=0;i<headers.size();i++) {
					std::map<std::string,std::string>::const_iterator p;
					if((p=env_.find(headers[i]))!=env_.end()) {
						remote_addr=p->second;
						break;
					}
				}
			}
			env_["REMOTE_HOST"] = env_["REMOTE_ADDR"] = remote_addr;

			if(request_uri_.empty() || request_uri_[0]!='/') {
				response("HTTP/1.0 400 Bad Request\r\n\r\n",h);
				return;
			}
			

			std::vector<std::string> script_names = 
				service().settings().get("http.script_names",std::vector<std::string>());

			std::string additional=service().settings().get("http.script","");
			
			if(!additional.empty())
				script_names.push_back(additional);


			for(unsigned i=0;i<script_names.size();i++) {

				std::string const &name=script_names[i];

				if(	request_uri_.size() >= name.size() 
					&& memcmp(request_uri_.c_str(),name.c_str(),name.size())==0)
				{
					env_["SCRIPT_NAME"]=name;
					size_t pos=request_uri_.find('?');
					if(pos==std::string::npos) {
						env_["PATH_INFO"]=util::urldecode(request_uri_.substr(name.size()));
					}
					else {
						env_["PATH_INFO"]=util::urldecode(request_uri_.substr(name.size(),pos-name.size()));
						env_["QUERY_STRING"]=request_uri_.substr(pos+1);
					}
					h(booster::system::error_code());
					return;
				}
			}
			
			size_t pos=request_uri_.find('?');
			if(pos==std::string::npos)
				env_["PATH_INFO"]=request_uri_;
			else {
				env_["PATH_INFO"]=request_uri_.substr(0,pos);
				env_["QUERY_STRING"]=request_uri_.substr(pos+1);
			}
			
			h(booster::system::error_code());

		}
		void response(char const *message,handler const &h)
		{
			socket_.async_write(io::buffer(message,strlen(message)),
					boost::bind(h,booster::system::error_code()));
		}

		bool parse_single_header(std::string const &header,std::string &name,std::string &value)
		{
			std::string::const_iterator p=header.begin(),e=header.end(),name_end;

			p=cppcms::http::protocol::skip_ws(p,e);
			name_end=cppcms::http::protocol::tocken(p,e);
			if(name_end==p)
				return false;
			name.assign(p,name_end);
			p=name_end;
			p=cppcms::http::protocol::skip_ws(p,e);
			if(p==e || *p!=':')
				return false;
			++p;
			p=cppcms::http::protocol::skip_ws(p,e);
			value.assign(p,e);
			for(unsigned i=0;i<name.size();i++) {
				if(name[i] == '-') 
					name[i]='_';
				else if('a' <= name[i] && name[i] <='z')
					name[i]=name[i]-'a' + 'A';
			}
			return true;
		}


		booster::shared_ptr<http> self()
		{
			return booster::static_pointer_cast<http>(shared_from_this());
		}
		
		friend class socket_acceptor<http>;
		
		booster::aio::socket socket_;

		std::vector<char> input_body_;
		unsigned input_body_ptr_;
		::cppcms::http::impl::parser input_parser_;
		std::vector<char> output_body_;
		unsigned output_body_ptr_;
		::cppcms::http::impl::parser output_parser_;


		std::map<std::string,std::string> env_;
		std::string script_name_;
		std::string response_line_;
		std::string request_method_;
		std::string request_uri_;
		bool headers_done_;
		bool first_header_observerd_;
		unsigned total_read_;
	};

	std::auto_ptr<acceptor> http_api_factory(cppcms::service &srv,std::string ip,int port,int backlog)
	{
		std::auto_ptr<acceptor> a(new socket_acceptor<http>(srv,ip,port,backlog));
		return a;
	}



} // cgi
} // impl
} // cppcms


