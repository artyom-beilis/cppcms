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
#include "cgi_api.h"
#include "cgi_acceptor.h"
#include <cppcms/service.h>
#include "service_impl.h"
#include "cppcms_error_category.h"
#include <cppcms/json.h>
#include <cppcms/cstdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <cppcms/config.h>

#include <booster/aio/buffer.h>

#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

#include "cached_settings.h"


#ifdef CPPCMS_WIN_NATIVE
#  ifndef NOMINMAX 
#    define NOMINMAX
#  endif
#  include <winsock2.h>
#else
#  include <arpa/inet.h>
#endif

namespace io = booster::aio;


namespace cppcms {
namespace impl {
namespace cgi {


	template<typename API> class socket_acceptor;
	class fastcgi : public connection {
	public:
		fastcgi(cppcms::service &srv) :
			connection(srv),
			socket_(srv.impl().get_io_service())
		{
			reset_all();
			int procs=srv.procs_no();
			if(procs < 1) procs=1;
			int threads=srv.threads_no();
			cuncurrency_hint_=srv.cached_settings().fastcgi.cuncurrency_hint;
			if(cuncurrency_hint_ < 0)
				cuncurrency_hint_ = procs*threads;
		}
		~fastcgi()
		{
		}
		virtual void async_read_headers(handler const &h)
		{
			reset_all();
			async_read_record(boost::bind(&fastcgi::on_start_request,self(),_1,h));
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
			if(read_length_ == content_length_) {
				socket_.get_io_service().post(boost::bind(
						h,
						booster::system::error_code(errc::protocol_violation,cppcms_category),
						0));
				return;
			}
			if(body_ptr_ < body_.size()) {
				size_t rest=body_.size() - body_ptr_;
				if(s > rest)
					s=rest;
				memcpy(p,&body_[body_ptr_],s);
				body_ptr_ += s;
				read_length_ += s;

				if(body_ptr_ == body_.size()) {
					body_ptr_ = 0;
					body_.clear();
				}

				if(read_length_ >= content_length_) {
					async_read_record(boost::bind(
						&fastcgi::on_read_stdin_eof_expected,
						self(),
						_1,
						h,
						s));
					return;
				}
				socket_.get_io_service().post(boost::bind(h,booster::system::error_code(),s));
				return;
			}
			else {
				async_read_record(boost::bind(
					&fastcgi::on_some_input_recieved,
					self(),
					_1,h,p,s));
				return;
			}
		}
	private:
		void on_some_input_recieved(booster::system::error_code const &e,io_handler const &h,void *p,size_t s)
		{
			if(e) { h(e,0); return; }
			if(	header_.type!=fcgi_stdin 
				|| header_.request_id!=request_id_ 
				|| header_.content_length==0)
			{
				h(booster::system::error_code(errc::protocol_violation,cppcms_category),0);
				return;
			}
			async_read_some(p,s,h);
		}
		void on_read_stdin_eof_expected(booster::system::error_code const &e,io_handler const &h,size_t s)
		{
			if(e) { h(e,s); return; }
			if(	header_.type!=fcgi_stdin 
				|| header_.request_id!=request_id_ 
				|| header_.content_length!=0)
			{
				h(booster::system::error_code(errc::protocol_violation,cppcms_category),s);
				return;
			}
			h(booster::system::error_code(),s);
		}
	public:	
		virtual void async_write_some(void const *p,size_t s,io_handler const &h)
		{
			booster::system::error_code dummy;
			do_write_some(p,s,h,true,dummy);
		}
		
		virtual size_t write_some(void const *buffer,size_t n,booster::system::error_code &e)
		{
			return do_write_some(buffer,n,io_handler(),false,e);
		}
		
		virtual size_t do_write_some(void const *p,size_t s,io_handler const &h,bool async,booster::system::error_code &e)
		{
			if(s==0) {
				if(async)
					socket_.get_io_service().post(boost::bind(h,booster::system::error_code(),0));
				return 0;
			}
			memset(&header_,0,sizeof(header_));
			header_.version=fcgi_version_1;
			header_.type=fcgi_stdout;
			header_.request_id=request_id_;
			if(s > 65535) s=65535;
			header_.content_length =s;
			header_.padding_length =(8 - (s % 8)) % 8;
			static char pad[8];
			
			header_.to_net();

			io::const_buffer packet = 
					io::buffer(&header_,sizeof(header_))
					+ io::buffer(p,s)
					+ io::buffer(pad,header_.padding_length);

			if(async) {
				socket_.async_write(
					packet,
					boost::bind(	h,
							_1,
							s));
				return s;
			}
			else {
				booster::system::error_code err;
				size_t res = socket_.write(packet,err);
				if(err && io::basic_socket::would_block(err)) {
					socket_.set_non_blocking(false);
					packet+=res;
					socket_.write(packet,e);
					return s;
				}
				else if(err) {
					e=err;
					return 0;
				}
				else
					return s;
			}
		}
		virtual booster::aio::io_service &get_io_service()
		{
			return socket_.get_io_service();
		}
		virtual bool keep_alive()
		{
			reset_all();
			return keep_alive_;
		}

		virtual void close()
		{
			booster::system::error_code e;
			socket_.close(e);
		}
		void prepare_eof()
		{
			memset(&eof_,0,sizeof(eof_));
			for(unsigned i=0;i<2;i++) {
				eof_.headers_[i].version=fcgi_version_1;
				eof_.headers_[i].request_id=request_id_;
			}
			eof_.headers_[0].type=fcgi_stdout;
			eof_.headers_[1].type=fcgi_end_request;
			eof_.headers_[1].content_length=8;
			eof_.record_.protocol_status=fcgi_request_complete;
			eof_.headers_[0].to_net();
			eof_.headers_[1].to_net();
			eof_.record_.to_net();
		}
		virtual void write_eof()
		{
			prepare_eof();	
			socket_.cancel();
			booster::system::error_code e;
			socket_.write(io::buffer(&eof_,sizeof(eof_)),e);
		}
		
		virtual void async_write_eof(handler const &h)
		{
			prepare_eof();	
			socket_.cancel();
			socket_.async_write(
				io::buffer(&eof_,sizeof(eof_)),
				boost::bind(	&fastcgi::on_written_eof,
						self(),
						_1,
						h));
		}

		virtual void on_written_eof(booster::system::error_code const &e,handler const &h)
		{
			if(e) { h(e); return; }

			// Stop reading from socket
			if(!keep_alive_) {
				booster::system::error_code err;
				socket_.shutdown(io::stream_socket::shut_rdwr,err);
			}

			h(booster::system::error_code());
		}

		// This is not really correct because server may try
		// to multiplex or ask control... But meanwhile it is good enough
		virtual void async_read_eof(callback const &h)
		{
			static char a;
			async_read_from_socket(&a,1,boost::bind(h));
		}
	
	private:

		// 
		//
		// defs
		//
		//

		struct fcgi_header {
			unsigned char version;
			unsigned char type;
			uint16_t request_id;
			uint16_t content_length;
			unsigned char padding_length;
			unsigned char reserverd;
			void to_host() { 
				request_id = ntohs(request_id);
				content_length = ntohs(content_length);
			}
			void to_net() { 
				request_id = htons(request_id);
				content_length = htons(content_length);
			}
		};

		enum {
			fcgi_header_len = 8,
			fcgi_version_1 = 1,
						

			fcgi_begin_request     = 1,
			fcgi_abort_request     = 2,
			fcgi_end_request       = 3,
			fcgi_params            = 4,
			fcgi_stdin             = 5,
			fcgi_stdout            = 6,
			fcgi_stderr            = 7,
			fcgi_data              = 8,
			fcgi_get_values        = 9,
			fcgi_get_values_result =10,
			fcgi_unknown_type      =11,
			fcgi_maxtype 	       =11,

			fcgi_null_request_id = 0
		};
		
		struct fcgi_request_body {
			uint16_t role;
			unsigned char flags;
			unsigned char reserved [5];
			void to_host() { role = ntohs(role); }
			void to_net() { role = htons(role); }
		};

		struct fcgi_begin_request_record {
			fcgi_header header;
			fcgi_request_body body;
			void to_host() { header.to_host(); body.to_host(); }
			void to_net() {  header.to_net(); body.to_net(); }
		};
		enum {
			fcgi_keep_conn = 1
		};
		enum {
			fcgi_responder = 1,
			fcgi_authorizer = 2,
			fcgi_filter = 3
		};

		struct fcgi_end_request_body {
			uint32_t app_status;
			unsigned char protocol_status;
			unsigned char reserved[3];
			void to_host() {  app_status = ntohl(app_status); }
			void to_net() { app_status = htonl(app_status); }
		}; 

		struct fcgi_end_request_record {
			fcgi_header header;
			fcgi_end_request_body body;
			void to_host() { header.to_host(); body.to_host(); }
			void to_net() {  header.to_net(); body.to_net(); }
		};

		enum {
			fcgi_request_complete = 0,
			fcgi_cant_mpx_conn    = 1,
			fcgi_overloaded       = 2,
			fcgi_unknown_role     = 3
		};


		struct fcgi_unknown_type_body {
			unsigned char type;
			unsigned char reserverd[7];
		};

		struct fcgi_unknown_type_record {
			fcgi_header header;
			fcgi_unknown_type_body body;
			void to_host() { header.to_host();}
			void to_net() {  header.to_net(); }
		};

		union fcgi_record {
			fcgi_header header;
			fcgi_begin_request_record begin ;
			fcgi_end_request_record  end;
			fcgi_unknown_type_record unknown;
		};
		
		//
		//
		// end of defs
		//
		//
		//
		//
	
		void on_start_request(booster::system::error_code const &e,handler const &h)
		{

			if(header_.version!=fcgi_version_1)
			{
				h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}
			if(header_.type==fcgi_get_values) {
				header_.type = fcgi_get_values_result;
				std::vector<std::pair<std::string,std::string> > pairs;
				pairs.reserve(4);
				if(!parse_pairs(pairs)) {
					h(booster::system::error_code(errc::protocol_violation,cppcms_category));
					return;
				}
				body_.clear();
				std::locale l("C");
				std::ostringstream ss;
				ss.imbue(l);
				ss<<cuncurrency_hint_;
				for(unsigned i=0;i<pairs.size();i++) {
					std::string name=pairs[i].first;
					if(name=="FCGI_MAX_CONNS" || name=="FCGI_MAX_REQS")
						add_pair(name,ss.str());
					else if(name=="FCGI_MPXS_CONNS")
						add_pair(name,"0");
				}
				async_send_respnse(boost::bind(	&fastcgi::on_params_response_sent,
								self(),
								_1,
								h));
			}
			else if(header_.type!=fcgi_begin_request) {
				async_read_headers(h);
				return;
			}
			
			if(body_.size()!=sizeof(fcgi_request_body)) {
				h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}
			fcgi_request_body *body=reinterpret_cast<fcgi_request_body*>(&body_.front());
			body->to_host();
			keep_alive_=body->flags & fcgi_keep_conn;
			
			if(body->role!=fcgi_responder) {
				header_.type=fcgi_end_request;
				body_.assign(0,8);
				fcgi_end_request_body *body=reinterpret_cast<fcgi_end_request_body*>(&body_.front());
				body->protocol_status=fcgi_unknown_role;
				body->to_net();
				async_send_respnse(boost::bind(	&fastcgi::on_params_response_sent,
								self(),
								_1,
								h));
				return;
			}
			request_id_=header_.request_id;
			
			body_.clear();
			if(non_blocking_read_record()) {
				params_record_expected(booster::system::error_code(),h);
			}
			else {
				async_read_record(boost::bind(&fastcgi::params_record_expected,self(),_1,h));
			}
		}

		uint32_t read_len(unsigned char const *&p,unsigned char const *e)
		{
			if (p<e && *p < 0x80) {
				return *p++;
			}
			else if(e-p >= 4) {
				uint32_t B3=*p++;
				uint32_t B2=*p++;
				uint32_t B1=*p++;
				uint32_t B0=*p++;
				uint32_t len = ((B3 & 0x7fU) << 24) + (B2 << 16) + (B1 << 8) + B0;
				return len;
			}
			else {
				return 0xFFFFFFFFu;
			}
		}

		void insert_to_container(std::map<std::string,std::string> &map,std::string &name,std::string &value)
		{
			map[name].swap(value);
		}
		void insert_to_container(std::vector<std::pair<std::string,std::string> > &v,std::string &name,std::string &value)
		{
			v.resize(v.size()+1);
			v.back().first.swap(name);
			v.back().second.swap(value);
		}

		template<typename Container>
		bool parse_pairs(Container &container)
		{
			unsigned char const *p=reinterpret_cast<unsigned char const *>(&body_.front());
			unsigned char const *e=p + body_.size();
			while(p<e) {
				uint32_t nlen=read_len(p,e);
				uint32_t vlen=read_len(p,e);
				if(nlen == 0xFFFFFFFFu || vlen == 0xFFFFFFFFu)
					return false;
				std::string name;
				if(uint32_t(e - p) >= nlen) { // don't chage order -- prevent integer overflow
					name.assign(reinterpret_cast<char const *>(p),nlen);
					p+=nlen;
				}
				else {
					return false;
				}
				std::string value;
				if(uint32_t(e - p) >= vlen) { // don't chage order -- prevent integer overflow
					value.assign(reinterpret_cast<char const *>(p),vlen);
					p+=vlen;
				}
				else {
					return false;
				}
				insert_to_container(container,name,value);
			}
			return true;
		}

		void params_record_expected(booster::system::error_code const &e,handler const &h)
		{
			for(;;){
				if(header_.type!=fcgi_params || header_.request_id!=request_id_)
					h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				if(header_.content_length!=0) { // eof
					if(body_.size() < 16384) { 
						if(non_blocking_read_record()) {
							continue;
						}
						else {
							async_read_record(boost::bind(	&fastcgi::params_record_expected,
											self(),
											_1,
											h));
						}
					}
					else {
						h(booster::system::error_code(errc::protocol_violation,cppcms_category));
					}
					return;
				}
				break;
			}


			parse_pairs(env_);

			body_.clear();

			std::map<std::string,std::string>::const_iterator p = env_.find("CONTENT_LENGTH");
			if(p==env_.end() || p->second.empty() || (content_length_ =  atoll(p->second.c_str())) <=0)
				content_length_=0;
			if(content_length_==0) {
				if(non_blocking_read_record()) {
					stdin_eof_expected(booster::system::error_code(),h);
				}
				else {
					async_read_record(boost::bind(
						&fastcgi::stdin_eof_expected,
						self(),
						_1,
						h));
				}
				return;
			}
			h(booster::system::error_code());
		}

		void stdin_eof_expected(booster::system::error_code const &e,handler const &h)
		{
			if(e) { h(e); return; }
			if(header_.type!=fcgi_stdin || header_.content_length!=0) {
				h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				return;				
			}
			h(booster::system::error_code());
		}

		void on_params_response_sent(booster::system::error_code const &e,handler const &h)
		{
			if(e) { h(e); return; }
			async_read_headers(h);
		}

		void async_send_respnse(handler const &h)
		{
			header_.content_length=body_.size();
			if(body_.size() % 8 != 0) {
				header_.padding_length=8 - (body_.size() % 8);
				body_.resize(body_.size() + header_.padding_length);
			
			}
			io::const_buffer packet =
					io::buffer(&header_,sizeof(header_))
					+ io::buffer(body_);
			
			header_.to_net();
			socket_.async_write(packet,boost::bind(h,_1));

		}

		bool non_blocking_read_record()
		{
			fcgi_header hdr;
			if(!peek_bytes(&hdr,sizeof(hdr)))
				return false;
			hdr.to_host();
			size_t buffer_size = get_buffer_size();
			
			if(buffer_size < sizeof(hdr) + hdr.content_length + hdr.padding_length)
				return false;
			skip_bytes(sizeof(hdr));
			header_ = hdr;
			size_t cur_size=body_.size();
			size_t rec_size=header_.content_length+header_.padding_length;
			if(rec_size==0) {
				return true;
			}
			body_.resize(cur_size + rec_size);
			read_bytes(&body_[cur_size],rec_size);
			body_.resize(cur_size + header_.content_length);
			return true;
		}
			
		void async_read_record(handler const &h)
		{
			async_read_from_socket(&header_,sizeof(header_),
					boost::bind(	&fastcgi::on_header_read,
							self(),
							_1,
							h));
		}
		
		struct on_header_read_binder : public booster::callable<void(booster::system::error_code const &,size_t)> {
			handler h;
			booster::shared_ptr<fastcgi> self;
			on_header_read_binder(handler const &_h,booster::shared_ptr<fastcgi> const &s) : h(_h),self(s)
			{
			}
			virtual void operator()(booster::system::error_code const &e,size_t read)
			{
				self->on_body_read(e,h);
			}
		};

		void on_header_read(booster::system::error_code const &e,handler const &h)
		{
			if(e) { h(e); return; }
			header_.to_host();
			size_t cur_size=body_.size();
			size_t rec_size=header_.content_length+header_.padding_length;
			if(rec_size==0) {
				h(booster::system::error_code());
				return;
			}
			body_.resize(cur_size + rec_size);
			std::auto_ptr<booster::callable<void(booster::system::error_code const &,size_t)> > cb;
			cb.reset(new on_header_read_binder(h,self()));
			async_read_from_socket(
				&body_[cur_size],rec_size,
				cb);
		}
		void on_body_read(booster::system::error_code const &e,handler const &h)
		{
			if(e) { h(e);  return; }
			body_.resize(body_.size() - header_.padding_length);
			h(booster::system::error_code());
		}

		void add_value(std::string const &value)
		{
			if(value.size() <128) {
				body_.push_back(char(value.size()));
			}
			else {
				uint32_t size=value.size();
				size |= 0x80000000u;
				size=htonl(size);
				char *p=(char*)&size;
				body_.insert(body_.end(),p,p+sizeof(size));
			}
		}
		void add_pair(std::string const &name,std::string const &value)
		{
			add_value(name);
			add_value(value);
			body_.insert(body_.end(),name.begin(),name.end());
			body_.insert(body_.end(),value.begin(),value.end());
		}


		booster::shared_ptr<fastcgi> self()
		{
			return booster::static_pointer_cast<fastcgi>(shared_from_this());
		}
		
		
		friend class socket_acceptor<fastcgi>;
		io::stream_socket socket_;

		fcgi_header header_;
		std::vector<char> body_;


		long long read_length_;
		long long content_length_;
		unsigned body_ptr_;
		int request_id_;
		bool keep_alive_;
		int cuncurrency_hint_;

		std::map<std::string,std::string> env_;
		struct eof_str {
			fcgi_header headers_[2];
			fcgi_end_request_body record_;
		} eof_;

		std::vector<char> cache_;
		size_t cache_start_,cache_end_;

		void reset_all()
		{
			memset(&header_,0,sizeof(header_));
			body_.clear();
			read_length_=content_length_=0;
			body_ptr_=0;
			request_id_=0;
			keep_alive_=false;
			env_.clear();
			memset(&eof_,0,sizeof(eof_));
			if(cache_.empty()) {
				cache_start_ = 0;
				cache_end_ = 0;
			}
		}

		bool peek_bytes(void *ptr,size_t n)
		{
			if(cache_end_ - cache_start_ >= n) {
				memcpy(ptr,&cache_[cache_start_],n);
				return true;
			}
			return false;
		}
		size_t get_buffer_size()
		{
			return cache_end_ - cache_start_;
		}
		void skip_bytes(size_t n)
		{
			assert(cache_end_ - cache_start_ >= n);
			cache_start_ +=n;
		}
		void read_bytes(void *ptr,size_t n)
		{
			assert(cache_end_ - cache_start_ >=n);
			memcpy(ptr,&cache_[cache_start_],n);
			cache_start_+=n;
		}

		void async_read_from_socket(void *ptr,size_t n,booster::callback<void(booster::system::error_code const &e,size_t read)> const &cb)
		{
			if(cache_end_ - cache_start_ >=n) {
				memcpy(ptr,&cache_[cache_start_],n);
				cache_start_+=n;
				socket_.get_io_service().post(boost::bind(cb,booster::system::error_code(),n));
				return;
			}
			if(cache_start_ == cache_end_) {
				cache_start_ = cache_end_ = 0;
			}
			else if(cache_start_!=0) {
				memmove(&cache_[0],&cache_[cache_start_],cache_end_ - cache_start_);
				cache_end_ = cache_end_ - cache_start_;
				cache_start_ = 0;
			}

			size_t min_size = std::max(n,size_t(16384));
			if(cache_.size() < n) {
				cache_.resize(min_size,0);
			}
			
			socket_.async_read_some(
				booster::aio::buffer(&cache_[cache_end_],cache_.size() - cache_end_),
					boost::bind(
						&fastcgi::on_some_read_from_socket,
						self(),
						_1,
						_2,
						cb,
						ptr,
						n));
		}
		void on_some_read_from_socket(	booster::system::error_code const &e,
						size_t read_size, 
						booster::callback<void(booster::system::error_code const &e,size_t read)> const &cb,
						void *ptr,
						size_t expected_read_size)
		{
			cache_end_ += read_size;
			if(e) {
				cb(e,0);
				return;
			}
			async_read_from_socket(ptr,expected_read_size,cb);
		}
	};

	std::auto_ptr<acceptor> fastcgi_api_tcp_socket_factory(cppcms::service &srv,std::string ip,int port,int backlog)
	{
		std::auto_ptr<acceptor> a(new socket_acceptor<fastcgi>(srv,ip,port,backlog));
		return a;
	}

#if !defined(CPPCMS_WIN32)

	std::auto_ptr<acceptor> fastcgi_api_unix_socket_factory(cppcms::service &srv,std::string socket,int backlog)
	{
		std::auto_ptr<acceptor> a(new socket_acceptor<fastcgi>(srv,socket,backlog));
		return a;
	}
	std::auto_ptr<acceptor> fastcgi_api_unix_socket_factory(cppcms::service &srv,int backlog)
	{
		std::auto_ptr<acceptor> a(new socket_acceptor<fastcgi>(srv,backlog));
		return a;
	}
#endif


} // cgi
} // impl
} // cppcms

