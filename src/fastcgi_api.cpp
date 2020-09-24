///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
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
#include <stdio.h>
#include <cppcms/config.h>

#include <booster/aio/buffer.h>
#include "binder.h"
#include <utility>
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


	template<typename API,typename Factory> class socket_acceptor;

	class fastcgi : public connection {
	public:
		fastcgi(cppcms::service &srv) :
			connection(srv),
			socket_(srv.impl().get_io_service()),
			header_(),
			full_header_(),
			eof_(),
			eof_callback_(false)
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
			if(socket_.native()!=io::invalid_socket) {
				booster::system::error_code e;
				socket_.shutdown(io::stream_socket::shut_rdwr,e);
			}
		}
		virtual void async_read_headers(handler const &h)
		{
			reset_all();
			async_read_record(mfunc_to_event_handler(&fastcgi::on_start_request,self(),h));
		}

		virtual void async_read_some(void *p,size_t s,io_handler const &h)
		{
			if(read_length_ == content_length_) {
				socket_.get_io_service().post(h,booster::system::error_code(errc::protocol_violation,cppcms_category),0);
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
					async_read_record(mfunc_to_event_handler(
						&fastcgi::on_read_stdin_eof_expected,
						self(),
						h,
						s));
					return;
				}
				socket_.get_io_service().post(h,booster::system::error_code(),s);
				return;
			}
			else {
				async_read_record(mfunc_to_event_handler(
					&fastcgi::on_some_input_recieved,
					self(),
					h,std::make_pair(p,s)));
				return;
			}
		}
		virtual void on_async_write_start(){}
		virtual void on_async_write_progress(bool){}
	private:
		void on_some_input_recieved(booster::system::error_code const &e,io_handler const &h,std::pair<void *,size_t> in)
		{
			if(e) { h(e,0); return; }
			if(	header_.type!=fcgi_stdin 
				|| header_.request_id!=request_id_ 
				|| header_.content_length==0)
			{
				h(booster::system::error_code(errc::protocol_violation,cppcms_category),0);
				return;
			}
			async_read_some(in.first,in.second,h);
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

		virtual booster::aio::const_buffer format_output(booster::aio::const_buffer const &in,bool completed,booster::system::error_code &)
		{
			booster::aio::const_buffer packet;
			booster::aio::const_buffer::entry const *chunks = in.get().first;
//#define DEBUG_FASTCGI
#ifdef DEBUG_FASTCGI
			{
				size_t n=in.get().second;
				printf("Format output of %d:\n",int(in.bytes_count()));
				for(size_t i=0;i<n;i++) {
					printf("[%.*s]",int(chunks[i].size),chunks[i].ptr);
				}
				if(completed) {
					printf("\nEOF\n");
				}
				else {
					printf("\n---\n");
				}
			}
#endif
			size_t reminder = in.bytes_count();
			size_t in_size = reminder;
			size_t chunk_consumed = 0;
			while(reminder > 0) {
				static const char pad[8]={0,0,0,0,0,0,0,0};
				static const size_t max_packet_len = 65535;
				size_t chunk = 0;
				int pad_len = 0;
				if(reminder > max_packet_len) {
					chunk = max_packet_len;
					if(in_size > max_packet_len && reminder == in_size) {
						// prepare only once
						full_header_.version = fcgi_version_1;
						full_header_.type=fcgi_stdout;
						full_header_.request_id=request_id_;
						full_header_.content_length = max_packet_len;
						full_header_.padding_length = pad_len = 1;
						full_header_.to_net();
					}
					else {
						pad_len = 1;
					}
					packet += io::buffer(&full_header_,sizeof(full_header_));
				}
				else {
					chunk = reminder;
					memset(&header_,0,sizeof(header_));
					header_.version=fcgi_version_1;
					header_.type=fcgi_stdout;
					header_.request_id=request_id_;
					header_.content_length = reminder;
					header_.padding_length =pad_len = (8 - (reminder % 8)) % 8;
					header_.to_net();

					packet += io::buffer(&header_,sizeof(header_));
				}

				reminder -= chunk;
				while(chunk > 0) {
					size_t next_size = chunks->size - chunk_consumed;
					if(next_size > chunk)
						next_size = chunk;

					packet += io::buffer(chunks->ptr + chunk_consumed, next_size);
					chunk_consumed += next_size;
					chunk -= next_size;
					if(chunk_consumed == chunks->size) {
						chunks++;
						chunk_consumed = 0;
					}
				}

				packet += io::buffer(pad,pad_len);
			}
			if(completed) {
				prepare_eof();
				packet += io::buffer(&eof_,sizeof(eof_));
			}
			#ifdef DEBUG_FASTCGI
			std::pair<booster::aio::const_buffer::entry const *,size_t> cnk = packet.get();
			for(size_t i=0;i<cnk.second;i++) {
				std::cerr << "[ " << (void const *)(cnk.first[i].ptr) << " " << cnk.first[i].size << "]\n" << std::endl;
			}
			#endif
			return packet;
		}
		virtual booster::aio::stream_socket &socket() { return socket_; }
		virtual booster::aio::io_service &get_io_service()
		{
			return socket_.get_io_service();
		}
		virtual bool keep_alive()
		{
			bool ka_value = keep_alive_;
			reset_all();
			return ka_value;
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
		virtual void do_eof()
		{
			if(eof_callback_)
				socket_.cancel();
			eof_callback_ = false;
		}

		struct io_handler_to_handler {
			callback h;
			io_handler_to_handler(callback const &c) : h(c) {}
			void operator()(booster::system::error_code const &,size_t)
			{
				h();
			}
		};

		struct io_handler_to_event_handler {
			handler h;
			io_handler_to_event_handler(handler const &c) : h(c) {}
			void operator()(booster::system::error_code const &e,size_t)
			{
				h(e);
			}
		};

		// This is not really correct because server may try
		// to multiplex or ask control... But meanwhile it is good enough
		virtual void async_read_eof(callback const &h)
		{
			eof_callback_ = true;
			static char a;
			async_read_from_socket(&a,1,io_handler_to_handler(h));
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
			unsigned char reserved;
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
			unsigned char reserved[7];
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
			if(e) {
				h(e);
				return;
			}
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
				async_send_respnse(mfunc_to_event_handler(&fastcgi::on_params_response_sent,
								self(),
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
				async_send_respnse(mfunc_to_event_handler(&fastcgi::on_params_response_sent,
								self(),
								h));
				return;
			}
			request_id_=header_.request_id;
			
			body_.clear();
			if(non_blocking_read_record()) {
				params_record_expected(booster::system::error_code(),h);
			}
			else {
				async_read_record(mfunc_to_event_handler(&fastcgi::params_record_expected,self(),h));
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

		bool parse_pairs()
		{
			unsigned char const *p=reinterpret_cast<unsigned char const *>(&body_.front());
			unsigned char const *e=p + body_.size();
			while(p<e) {
				uint32_t nlen=read_len(p,e);
				uint32_t vlen=read_len(p,e);
				if(nlen == 0xFFFFFFFFu || vlen == 0xFFFFFFFFu)
					return false;
				char *name=0;
				if(uint32_t(e - p) >= nlen) { // don't chage order -- prevent integer overflow
					name = pool_.add(reinterpret_cast<char const *>(p),nlen);
					p+=nlen;
				}
				else {
					return false;
				}
				char *value = 0;
				if(uint32_t(e - p) >= vlen) { // don't chage order -- prevent integer overflow
					value = pool_.add(reinterpret_cast<char const *>(p),vlen);
					p+=vlen;
				}
				else {
					return false;
				}
				env_.add(name,value);
			}
			return true;
		}

		bool parse_pairs(std::vector<std::pair<std::string,std::string> > &container)
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
				container.resize(container.size()+1);
				container.back().first.swap(name);
				container.back().second.swap(value);
			}
			return true;
		}

		void params_record_expected(booster::system::error_code const &e,handler const &h)
		{
			if(e) {
				h(e);
				return;
			}
			for(;;){
				if(header_.type!=fcgi_params || header_.request_id!=request_id_)
					h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				if(header_.content_length!=0) { // eof
					if(body_.size() < 16384) { 
						if(non_blocking_read_record()) {
							continue;
						}
						else {
							async_read_record(mfunc_to_event_handler(&fastcgi::params_record_expected,
											self(),
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


			parse_pairs();

			body_.clear();

			char const *s_length = env_.get("CONTENT_LENGTH");
			if(!s_length || *s_length == 0 || (content_length_ =  atoll(s_length)) <=0)
				content_length_=0;
			if(content_length_==0) {
				if(non_blocking_read_record()) {
					stdin_eof_expected(booster::system::error_code(),h);
				}
				else {
					async_read_record(mfunc_to_event_handler(
						&fastcgi::stdin_eof_expected,
						self(),
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

		// this is internal function for short messages
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
			socket_.async_write(packet,io_handler_to_event_handler(h));

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
					mfunc_to_io_handler(
							&fastcgi::on_header_read,
							self(),
							h));
		}
		
		struct on_header_read_binder : public booster::callable<void(booster::system::error_code const &,size_t)> {
			handler h;
			booster::shared_ptr<fastcgi> self;
			on_header_read_binder(handler const &_h,booster::shared_ptr<fastcgi> const &s) : h(_h),self(s)
			{
			}
			virtual void operator()(booster::system::error_code const &e,size_t /*read*/)
			{
				self->on_body_read(e,h);
			}
		};

		void on_header_read(booster::system::error_code const &e,size_t /*unused read*/,handler const &h)
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
			std::unique_ptr<booster::callable<void(booster::system::error_code const &,size_t)> > cb;
			cb.reset(new on_header_read_binder(h,self()));
			async_read_from_socket(
				&body_[cur_size],rec_size,
				std::move(cb));
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
		fcgi_header full_header_;
		std::vector<char> body_;


		long long read_length_;
		long long content_length_;
		unsigned body_ptr_;
		int request_id_;
		bool keep_alive_;
		int cuncurrency_hint_;

		struct eof_str {
			fcgi_header headers_[2];
			fcgi_end_request_body record_;
		} eof_;

		std::vector<char> cache_;
		size_t cache_start_,cache_end_;
		bool eof_callback_;

		void reset_all()
		{
			memset(&header_,0,sizeof(header_));
			body_.clear();
			read_length_=content_length_=0;
			body_ptr_=0;
			request_id_=0;
			keep_alive_=false;
			env_.clear();
			pool_.clear();
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

		void async_read_from_socket(void *ptr,size_t n,booster::aio::io_handler const &cb)
		{
			if(cache_end_ - cache_start_ >=n) {
				memcpy(ptr,&cache_[cache_start_],n);
				cache_start_+=n;
				socket_.get_io_service().post(cb,booster::system::error_code(),n);
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
					mfunc_to_io_handler(
						&fastcgi::on_some_read_from_socket,
						self(),
						cb,
						std::make_pair(ptr,n)));
		}
		void on_some_read_from_socket(	booster::system::error_code const &e,
						size_t read_size, 
						booster::callback<void(booster::system::error_code const &e,size_t read)> const &cb,
						std::pair<void *,size_t> inp)
		{
			void *ptr = inp.first;
			size_t expected_read_size = inp.second;
			cache_end_ += read_size;
			if(e) {
				cb(e,0);
				return;
			}
			async_read_from_socket(ptr,expected_read_size,cb);
		}
	};

	std::unique_ptr<acceptor> fastcgi_api_tcp_socket_factory(cppcms::service &srv,std::string ip,int port,int backlog)
	{
		return std::unique_ptr<acceptor>(new socket_acceptor<fastcgi>(srv,ip,port,backlog));
	}

#if !defined(CPPCMS_WIN32)

	std::unique_ptr<acceptor> fastcgi_api_unix_socket_factory(cppcms::service &srv,std::string socket,int backlog)
	{
		return std::unique_ptr<acceptor>(new socket_acceptor<fastcgi>(srv,socket,backlog));
	}
	std::unique_ptr<acceptor> fastcgi_api_unix_socket_factory(cppcms::service &srv,int backlog)
	{
		return std::unique_ptr<acceptor>(new socket_acceptor<fastcgi>(srv,backlog));
	}
#endif


} // cgi
} // impl
} // cppcms

