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
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif
#include <booster/aio/buffer.h>
#include <string.h>

namespace io = booster::aio;

namespace cppcms {
namespace impl {
namespace cgi {
	template<typename API,typename Factory> class socket_acceptor;
	class scgi : public connection {
	public:
		scgi(cppcms::service &srv) :
			connection(srv),
			start_(0),
			end_(0),
			socket_(srv.impl().get_io_service()),
			eof_callback_(false)
		{
		}
		~scgi()
		{
			if(socket_.native()!=io::invalid_socket) {
				booster::system::error_code e;
				socket_.shutdown(io::stream_socket::shut_rdwr,e);
			}
		}
		virtual void async_read_headers(handler const &h)
		{
			buffer_.resize(16);
			socket_.async_read(
				io::buffer(buffer_),
				boost::bind(
					&scgi::on_first_read,
					self(),
					_1,_2,
					h));

		}

		void on_first_read(booster::system::error_code const &e,size_t n,handler const &h)
		{
			if(e) {
				h(e);
				return;
			}
			sep_=std::find(buffer_.begin(),buffer_.begin()+n,':') - buffer_.begin();
			if(sep_ >= 16) {
				h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}
			buffer_[sep_]=0;
			int len=atoi(&buffer_.front());
			if(len < 0 || 16384 < len) {
				h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}
			size_t size=n;
			buffer_.resize(sep_ + 2 + len); // len of number + ':' + content + ','
			if(buffer_.size() <= size) {
				// It can't be so short so
				h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}
			socket_.async_read(
				io::buffer(&buffer_[size],buffer_.size() - size),
				boost::bind(	&scgi::on_headers_chunk_read,
						self(),
						_1,
						h));
		}
		void on_headers_chunk_read(booster::system::error_code const &e,handler const &h)
		{
			if(e) { h(e); return; }
			if(buffer_.back()!=',') {
				buffer_.back() = 0;
				// make sure it is NUL terminated
				h(booster::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}

			char const *p=&buffer_[sep_ + 1];
			while(p < &buffer_.back()) {
				char *key=pool_.add(p);
				p+=strlen(p)+1;
				if(p>=&buffer_.back())
					break;
				char *value=pool_.add(p);
				p+=strlen(p)+1;
				env_.add(key,value);
			}
			buffer_.clear();

			h(booster::system::error_code());
		}
		virtual booster::aio::const_buffer format_output(booster::aio::const_buffer const &in,bool /*comleted*/,booster::system::error_code &/*e*/) 
		{
			return in;
		}

		virtual void async_read_some(void *p,size_t s,io_handler const &h)
		{
			socket_.async_read_some(io::buffer(p,s),h);
		}
		virtual booster::aio::io_service &get_io_service()
		{
			return socket_.get_io_service();
		}
		virtual bool keep_alive()
		{
			return false;
		}

		virtual void do_eof()
		{
			if(eof_callback_)
				socket_.cancel();
			eof_callback_ = false;
			booster::system::error_code e;
			socket_.shutdown(io::stream_socket::shut_wr,e);
			socket_.close(e);
		}

		virtual void async_read_eof(callback const &h)
		{
			eof_callback_ = true;
			static char a;
			socket_.async_read_some(io::buffer(&a,1),boost::bind(h));
		}

		virtual void on_some_output_written() {} 
		virtual booster::aio::stream_socket &socket() { return socket_; }
	private:
		size_t start_,end_,sep_;
		booster::shared_ptr<scgi> self()
		{
			return booster::static_pointer_cast<scgi>(shared_from_this());
		}
		friend class socket_acceptor<scgi>;
		io::stream_socket socket_;
		std::vector<char> buffer_;
		bool eof_callback_;
	};

	
	
	std::auto_ptr<acceptor> scgi_api_tcp_socket_factory(cppcms::service &srv,std::string ip,int port,int backlog)
	{
		std::auto_ptr<acceptor> a(new socket_acceptor<scgi>(srv,ip,port,backlog));
		return a;
	}
#if !defined(CPPCMS_WIN32)
	std::auto_ptr<acceptor> scgi_api_unix_socket_factory(cppcms::service &srv,std::string socket,int backlog)
	{
		std::auto_ptr<acceptor> a(new socket_acceptor<scgi>(srv,socket,backlog));
		return a;
	}
	std::auto_ptr<acceptor> scgi_api_unix_socket_factory(cppcms::service &srv,int backlog)
	{
		std::auto_ptr<acceptor> a(new socket_acceptor<scgi>(srv,backlog));
		return a;
	}
#endif


} // cgi
} // impl
} // cppcms


