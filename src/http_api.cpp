///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
//#define DEBUG_HTTP_PARSER
#include "cgi_api.h"
#include "cgi_acceptor.h"
#include <cppcms/service.h>
#include "service_impl.h"
#include "cppcms_error_category.h"
#include <cppcms/json.h>
#include "http_parser.h"
#include <cppcms/config.h>
#include <cppcms/util.h>
#include <string.h>
#include <iostream>
#include <list>
#include <sstream>
#include <algorithm>
#include <cppcms_boost/bind.hpp>

namespace boost = cppcms_boost;

#if defined(__sun)  
#  ifndef CPPCMS_NO_SO_SNDTIMO
#    define CPPCMS_NO_SO_SNDTIMO
#  endif
#endif

#ifdef CPPCMS_NO_SO_SNDTIMO
#include <poll.h>
#include <errno.h>
#endif


#include <booster/aio/buffer.h>
#include <booster/aio/deadline_timer.h>
#include <booster/aio/aio_category.h>
#include <booster/log.h>

#include "cached_settings.h"
#include "format_number.h"
#include "string_map.h"
#include "send_timeout.h"
#include "rewrite.h"

namespace io = booster::aio;



namespace cppcms {
namespace impl {
namespace cgi {

	class http;

	class http_watchdog : public booster::enable_shared_from_this<http_watchdog> {
	public:
		typedef booster::shared_ptr<http> http_ptr;
		typedef booster::weak_ptr<http> weak_http_ptr;

		http_watchdog(booster::aio::io_service &srv) :
			timer_(srv)
		{
		}

		void check(booster::system::error_code const &e=booster::system::error_code());

		void add(weak_http_ptr p)
		{
			connections_.insert(p);
		}

		void remove(weak_http_ptr p)
		{
			connections_.erase(p);
		}

	private:
		typedef std::set<weak_http_ptr> connections_type;
		connections_type connections_;
		booster::aio::deadline_timer timer_;
	};
	
	class http_creator {
	public:
		http_creator() {} // for concept
		http_creator(booster::aio::io_service &srv,json::value const &settings,std::string const &ip="0.0.0.0",int port = 8080) :
			ip_(ip),port_(port),watchdog_(new http_watchdog(srv))
		{
			if(settings.find("http.rewrite").type()==json::is_array) {
				rewrite_.reset(new url_rewriter(settings.find("http.rewrite").array()));
			}
			watchdog_->check();
		}
		http *operator()(cppcms::service &srv) const;
	private:
		std::string ip_;
		int port_;
		booster::shared_ptr<http_watchdog> watchdog_;
		booster::shared_ptr<url_rewriter> rewrite_;
	};


	namespace {
		char non_const_empty_string[1]={0};
	}
	class http : public connection {
	public:
		http(	cppcms::service &srv,
			std::string const &ip,
			int port,
			booster::shared_ptr<http_watchdog> wd,
			booster::shared_ptr<url_rewriter> rw
			) 
		:
			connection(srv),
			socket_(srv.impl().get_io_service()),
			input_body_ptr_(0),
			input_parser_(input_body_,input_body_ptr_),
			output_body_ptr_(0),
			output_parser_(output_body_,output_body_ptr_),
			request_method_(non_const_empty_string),
			request_uri_(non_const_empty_string),
			headers_done_(false),
			first_header_observerd_(false),
			total_read_(0),
			time_to_die_(0),
			timeout_(0),
			sync_option_is_set_(false),
			in_watchdog_(false),
			watchdog_(wd),
			rewrite_(rw)
		{

			env_.add("SERVER_SOFTWARE",CPPCMS_PACKAGE_NAME "/" CPPCMS_PACKAGE_VERSION);
			env_.add("SERVER_NAME",pool_.add(ip));
			char *sport = pool_.alloc(10);
			format_number(port,sport,10);
			env_.add("SERVER_PORT",sport);
			env_.add("GATEWAY_INTERFACE","CGI/1.0");
			env_.add("SERVER_PROTOCOL","HTTP/1.0");
			timeout_ = srv.cached_settings().http.timeout;

		}
		~http()
		{
			if(socket_.native()!=io::invalid_socket) {
				booster::system::error_code e;
				socket_.shutdown(io::stream_socket::shut_rdwr,e);
			}
		}

		void update_time()
		{
			time_to_die_ = time(0) + timeout_;
		}

		void log_timeout()
		{
			char const *uri = request_uri_;
			if(!uri || *uri==0)
				uri = "unknown";
			booster::system::error_code e;
			BOOSTER_INFO("cppcms_http") << "Timeout on connection for URI: " << uri << " from " << socket_.remote_endpoint(e).ip();
		}

		void die()
		{
			log_timeout();
			close();
		}
		void async_die()
		{
			socket_.cancel();
			die();
		}

		time_t time_to_die()
		{
			return time_to_die_;
		}

		void async_read_some_headers(handler const &h)
		{
			socket_.on_readable(boost::bind(&http::some_headers_data_read,self(),_1,h));
			update_time();
		}
		virtual void async_read_headers(handler const &h)
		{
			update_time();
			add_to_watchdog();
			async_read_some_headers(h);
		}

		void some_headers_data_read(booster::system::error_code const &er,handler const &h)
		{
			if(er) { h(er); return; }

			booster::system::error_code e;
			size_t n = socket_.bytes_readable(e);
			if(e) { h(e); return ; }

			if(n > 16384)
				n=16384;
			if(input_body_.capacity() < n) {
				input_body_.reserve(n);
			}
			input_body_.resize(input_body_.capacity(),0);
			input_body_ptr_=0;
			
			n = socket_.read_some(booster::aio::buffer(input_body_),e);

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
					async_read_some_headers(h);
					return;
				case parser::got_header:
					if(!first_header_observerd_) {
						first_header_observerd_=true;
						char const *header_begin = input_parser_.header_.c_str();
						char const *header_end   = header_begin + input_parser_.header_.size();
						char const *query=header_end;
						char const *rmethod=std::find(	header_begin,
										header_end,
										' ');
						if(rmethod!=header_end) 
							query=std::find(rmethod+1,header_end,' ');
						if(query!=header_end) {
							request_method_ = pool_.add(header_begin,rmethod);
							request_uri_ = pool_.add(rmethod+1,query);
							first_header_observerd_=true;
							BOOSTER_INFO("cppcms_http") << request_method_ <<" " << request_uri_;
						}
						else {
							h(booster::system::error_code(errc::protocol_violation,cppcms_category));
							return;
						}
					}
					else { // Any other header
						char const *name = "";
						char const *value = "";
						if(!parse_single_header(input_parser_.header_,name,value))  {
							h(booster::system::error_code(errc::protocol_violation,cppcms_category));
							return;
						}
						if(strcmp(name,"CONTENT_LENGTH")==0 || strcmp(name,"CONTENT_TYPE")==0)
							env_.add(name,value);
						else {
							char *updated_name =pool_.alloc(strlen(name) + 5 + 1);
							strcpy(updated_name,"HTTP_");
							strcat(updated_name,name);
							env_.add(updated_name,value);
						}
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

		virtual void async_read_some(void *p,size_t s,io_handler const &h)
		{
			update_time();
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
			if(input_body_.capacity()!=0) {
				std::vector<char> v;
				input_body_.swap(v);
			}
			socket_.async_read_some(io::buffer(p,s),h);
		}
		virtual void async_write_eof(handler const &h)
		{
			remove_from_watchdog();
			booster::system::error_code e;
			socket_.shutdown(io::stream_socket::shut_wr,e);
			socket_.get_io_service().post(h,booster::system::error_code());
		}
		virtual void write_eof()
		{
			booster::system::error_code e;
			socket_.shutdown(io::stream_socket::shut_wr,e);
			socket_.close(e);
		}
		virtual void async_write(void const *p,size_t s,io_handler const &h)
		{
			update_time();
			watchdog_->add(self());
			if(headers_done_)
				socket_.async_write(io::buffer(p,s),h);
			else
				process_output_headers(p,s,h);
		}
		virtual size_t write(void const *p,size_t n,booster::system::error_code &e)
		{
			if(headers_done_) 
				return write_to_socket(io::buffer(p,n),e);	
			return process_output_headers(p,n);
		}
		#ifndef CPPCMS_NO_SO_SNDTIMO
		// using SO_SNDTIMO on almost all platforms
		size_t write_to_socket(booster::aio::const_buffer const &buf,booster::system::error_code &e)
		{
			set_sync_options(e);
			if(e) return 0;
			booster::ptime start = booster::ptime::now();
			size_t n = socket_.write(buf,e);
			booster::ptime end = booster::ptime::now();
			// it may actually return with success but return small
			// a small buffer
			if(booster::ptime::to_number(end - start) >= timeout_ - 0.1) {
				die(); 
				return n;
			}
			if(e  && (io::basic_socket::would_block(e) 
				#ifdef CPPCMS_WIN32
				|| e.value() == 10060   // WSAETIMEDOUT - do not want to include windows.h
				#endif
				)
				) { 
				// handle timeout with SO_SNDTIMEO
				// that responds with EAGIAN or EWOULDBLOCK
				die();
			}
			return n;
		}
		#else 
		// we need to fallback to poll, fortunatelly only on some Unixes like Solaris, so we can use poll(2)
		size_t write_to_socket(booster::aio::const_buffer const &bufin,booster::system::error_code &e)
		{
			booster::aio::const_buffer buf = bufin;
			size_t total = 0;
			while(!buf.empty()) {
				size_t n = socket_.write_some(buf,e);
				if(!e || !io::basic_socket::would_block(e)) {
					buf += n
					total += n;
					continue;
				}
				booster::ptime start = booster::ptime::now();

				pollfd pfd=pollfd();
				pfd.fd = socket_.native();
				pfd.events = POLLOUT;
				int msec = timeout_ * 1000;
				int msec_total = msec;
				for(;;) {
					int r = poll(&pfd,1,msec);

					// handle restart after EINTR
					if(r < 0) {
						if(errno == EINTR) {
							int passed = int(booster::ptime::to_number(booster::ptime::now() - start)*1000);
							msec = msec_total - passed; 
							if(msec < 0)
								msec = 0;
							continue;
						}
						e = booster::system::error_code(errno,booster::system::system_category);
						return total;
					}
					else if(r == 0) {
						// timeout :-(
						e=booster::system::error_code(errc::protocol_violation,cppcms_category);
						die();
						return total;
					}
				}
				// check if we can write
				if(pfd.revents & POLLOUT) {
					e = booster::system::error_code();
					n = socket_.write_some(buf,e);
					// restart polling if we get would_block again
					if(n == 0 && io::basic_socket::would_block(e))
						continue;
					buf += n;
					total += n;
				}
				else {
					e=booster::system::error_code(
						booster::aio::aio_error::select_failed,
						booster::aio::aio_error_cat);
					return total;
				}
			} // while
		}
		#endif
		void set_sync_options(booster::system::error_code &e)
		{
			if(!sync_option_is_set_) {
				socket_.set_non_blocking(false,e);
				if(e)
					return;
				cppcms::impl::set_send_timeout(socket_,timeout_,e);
				if(e)
					return;
				sync_option_is_set_ = true;
			}

		}
		virtual booster::aio::io_service &get_io_service()
		{
			return socket_.get_io_service();
		}
		virtual bool keep_alive()
		{
			return false;
		}

		void close()
		{
			booster::system::error_code e;
			socket_.shutdown(io::stream_socket::shut_rdwr,e);
			socket_.close(e);
		}
		virtual void async_read_eof(callback const &h)
		{
			watchdog_->add(self());
			static char a;
			socket_.async_read_some(io::buffer(&a,1),boost::bind(h));
		}

	private:
		size_t process_output_headers(void const *p,size_t s,io_handler const &h=io_handler())
		{
			char const *ptr=static_cast<char const *>(p);
			output_body_.insert(output_body_.end(),ptr,ptr+s);

			using cppcms::http::impl::parser;

			for(;;) {
				switch(output_parser_.step()) {
				case parser::more_data:
					if(h)
						h(booster::system::error_code(),s);
					return s;
				case parser::got_header:
					{
						char const *name="";
						char const *value="";
						if(!parse_single_header(output_parser_.header_,name,value))  {
							h(booster::system::error_code(errc::protocol_violation,cppcms_category),s);
							return s;
						}
						if(strcmp(name,"STATUS")==0) {
							response_line_ = "HTTP/1.0 ";
							response_line_ +=value;
							response_line_ +="\r\n";
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
				"Server: CppCMS-Embedded/" CPPCMS_PACKAGE_VERSION "\r\n"
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
			if(!h) {
				booster::system::error_code e;
				write_to_socket(packet,e);
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
			if(	strcmp(request_method_,"GET")!=0 
				&& strcmp(request_method_,"POST")!=0
				&& strcmp(request_method_,"HEAD")!=0
				&& strcmp(request_method_,"PUT")!=0
				&& strcmp(request_method_,"DELETE")!=0
				&& strcmp(request_method_,"OPTIONS")!=0
			  ) 
			{
				error_response("HTTP/1.0 501 Not Implemented\r\n\r\n",h);
				return;
			}

			env_.add("REQUEST_METHOD",request_method_);

			if(rewrite_)
				request_uri_ = rewrite_->rewrite(request_uri_,pool_);

			char const *remote_addr=0;
			
			if(service().cached_settings().http.proxy.behind==true) {
				std::vector<std::string> const &variables = 
					service().cached_settings().http.proxy.remote_addr_cgi_variables;

				for(unsigned i=0;remote_addr == 0 && i<variables.size();i++) {
					remote_addr = env_.get(variables[i].c_str());
				}
			}
			
			if(!remote_addr) {
				booster::system::error_code e;
				remote_addr=pool_.add(socket_.remote_endpoint(e).ip());
				if(e) {
					close();
					h(e);
					return;
				}
			}
			
			env_.add("REMOTE_HOST",remote_addr);
			env_.add("REMOTE_ADDR",remote_addr);

			if(request_uri_[0]!='/') {
				error_response("HTTP/1.0 400 Bad Request\r\n\r\n",h);
				return;
			}
			
			char *path=non_const_empty_string;
			char *query = strchr(request_uri_,'?');
			if(query == 0) {
				path=request_uri_;
			}
			else {
				path=pool_.add(request_uri_,query - request_uri_);
				env_.add("QUERY_STRING",query + 1);
			}
			
			std::vector<std::string> const &script_names = 
				service().cached_settings().http.script_names;

			size_t path_size = strlen(path);
			for(unsigned i=0;i<script_names.size();i++) {
				std::string const &name=script_names[i];
				size_t name_size = name.size();
				if(path_size >= name_size && memcmp(path,name.c_str(),name_size) == 0 
				   && (path_size == name_size || path[name_size]=='/'))
				{
					env_.add("SCRIPT_NAME",pool_.add(name));
					path = path + name_size;
					break;
				}
			}
			
			env_.add("PATH_INFO",pool_.add(util::urldecode(path,path+strlen(path)))); 

			update_time();
			h(booster::system::error_code());

		}
		void on_async_read_complete()
		{
			remove_from_watchdog();
		}
		void error_response(char const *message,handler const &h)
		{
			socket_.async_write(io::buffer(message,strlen(message)),
					boost::bind(&http::on_error_response_written,self(),_1,h));
		}
		void on_error_response_written(booster::system::error_code const &e,handler const &h)
		{
			if(e) {
				h(e);
				return;
			}
			close();
			h(booster::system::error_code(errc::protocol_violation,cppcms_category));
		}

		bool parse_single_header(std::string const &header,char const *&o_name,char const *&o_value)
		{
			char const *p=header.c_str();
			char const *e=p + header.size();
			char const *name_end = p;

			p=cppcms::http::protocol::skip_ws(p,e);
			name_end=cppcms::http::protocol::tocken(p,e);
			if(name_end==p)
				return false;
			size_t name_size = name_end - p;
			char *name = pool_.alloc(name_size + 1);
			*std::copy(p,name_end,name) = 0;
			p=name_end;
			p=cppcms::http::protocol::skip_ws(p,e);
			if(p==e || *p!=':')
				return false;
			++p;
			p=cppcms::http::protocol::skip_ws(p,e);
			char *value = pool_.alloc(e-p+1);
			*std::copy(p,e,value) = 0;
			for(unsigned i=0;i<name_size;i++) {
				if(name[i] == '-') 
					name[i]='_';
				else if('a' <= name[i] && name[i] <='z')
					name[i]=name[i]-'a' + 'A';
			}
			o_name = name;
			o_value = value;
			return true;
		}


		booster::shared_ptr<http> self()
		{
			return booster::static_pointer_cast<http>(shared_from_this());
		}
		
		friend class socket_acceptor<http,http_creator>;
		
		booster::aio::stream_socket socket_;

		std::vector<char> input_body_;
		unsigned input_body_ptr_;
		::cppcms::http::impl::parser input_parser_;
		std::vector<char> output_body_;
		unsigned output_body_ptr_;
		::cppcms::http::impl::parser output_parser_;


		std::string response_line_;
		char *request_method_;
		char *request_uri_;
		bool headers_done_;
		bool first_header_observerd_;
		unsigned total_read_;
		time_t time_to_die_;
		int timeout_;
		bool sync_option_is_set_;
		bool in_watchdog_;

		void add_to_watchdog()
		{
			if(!in_watchdog_) {
				watchdog_->add(self());
				in_watchdog_ = true;
			}
		}
		void remove_from_watchdog()
		{
			if(in_watchdog_) {
				watchdog_->remove(self());
				in_watchdog_ = false;
			}
		}

		booster::shared_ptr<http_watchdog> watchdog_;
		booster::shared_ptr<url_rewriter> rewrite_;
	};
	void http_watchdog::check(booster::system::error_code const &e)
	{
		if(e)
			return;

		std::list<http_ptr> kill;

		time_t now = time(0);

		for(connections_type::iterator p=connections_.begin(),e=connections_.end();p!=e;) {
			booster::shared_ptr<http> ptr = p->lock();
			if(!ptr) {
				connections_type::iterator tmp = p;
				++p;
				connections_.erase(tmp);
			}
			else {
				if(ptr->time_to_die() < now) {
					kill.push_back(ptr);
					connections_type::iterator tmp = p;
					++p;
					connections_.erase(tmp);
				}
				else {
					++p;
				}
			}
		}

		for(std::list<http_ptr>::iterator p=kill.begin();p!=kill.end();++p) {
			(*p)->async_die();
		}

		timer_.expires_from_now(booster::ptime(1));
		timer_.async_wait(boost::bind(&http_watchdog::check,shared_from_this(),_1));
	}

	http *http_creator::operator()(cppcms::service &srv) const
	{
		return new http(srv,ip_,port_,watchdog_,rewrite_);
	}

	std::auto_ptr<acceptor> http_api_factory(cppcms::service &srv,std::string ip,int port,int backlog)
	{
		typedef socket_acceptor<http,http_creator> acceptor_type;
		std::auto_ptr<acceptor_type> acc(new acceptor_type(srv,ip,port,backlog));
		acc->factory(http_creator(srv.get_io_service(),srv.settings(),ip,port));
		std::auto_ptr<acceptor> a(acc);
		return a;
	}



} // cgi
} // impl
} // cppcms


