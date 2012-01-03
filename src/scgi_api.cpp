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
			socket_(srv.impl().get_io_service())
		{
		}
		~scgi()
		{
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

		virtual void async_read_some(void *p,size_t s,io_handler const &h)
		{
			socket_.async_read_some(io::buffer(p,s),h);
		}
		virtual void async_write_some(void const *p,size_t s,io_handler const &h)
		{
			socket_.async_write_some(io::buffer(p,s),h);
		}
		virtual size_t write_some(void const *buffer,size_t n,booster::system::error_code &e)
		{
			booster::system::error_code err;
			size_t res = socket_.write_some(io::buffer(buffer,n),err);
			if(err && io::basic_socket::would_block(err)) {
				socket_.set_non_blocking(false);
				return socket_.write_some(io::buffer(buffer,n),e);
			}
			else if(err) {
				e=err;
				return res;
			}
			else 
				return res;
		}
		virtual booster::aio::io_service &get_io_service()
		{
			return socket_.get_io_service();
		}
		virtual bool keep_alive()
		{
			return false;
		}

		virtual void write_eof()
		{
			booster::system::error_code e;
			socket_.shutdown(io::stream_socket::shut_wr,e);
			socket_.close(e);
		}
		virtual void async_write_eof(handler const &h)
		{
			booster::system::error_code e;
			socket_.shutdown(io::stream_socket::shut_wr,e);
			socket_.get_io_service().post(boost::bind(h,booster::system::error_code()));
		}

		virtual void close()
		{
			booster::system::error_code e;
			socket_.shutdown(io::stream_socket::shut_rd,e);
			socket_.close(e);
		}
		
		virtual void async_read_eof(callback const &h)
		{
			static char a;
			socket_.async_read_some(io::buffer(&a,1),boost::bind(h));
		}

	private:
		size_t start_,end_,sep_;
		booster::shared_ptr<scgi> self()
		{
			return booster::static_pointer_cast<scgi>(shared_from_this());
		}
		friend class socket_acceptor<scgi>;
		io::stream_socket socket_;
		std::vector<char> buffer_;
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


