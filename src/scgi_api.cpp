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

namespace io = booster::aio;

namespace cppcms {
namespace impl {
namespace cgi {
	template<typename API> class socket_acceptor;
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
			booster::system::error_code e;
			socket_.shutdown(io::socket::shut_rdwr,e);
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
				std::string key=p;
				p+=key.size()+1;
				if(p>=&buffer_.back())
					break;
				std::string value=p;
				p+=value.size()+1;
				env_.insert(std::pair<std::string,std::string>(key,value));
			}
			buffer_.clear();

			h(booster::system::error_code());
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
			socket_.async_read_some(io::buffer(p,s),h);
		}
		virtual void async_write_some(void const *p,size_t s,io_handler const &h)
		{
			socket_.async_write_some(io::buffer(p,s),h);
		}
		virtual size_t write_some(void const *buffer,size_t n)
		{
			return socket_.write_some(io::buffer(buffer,n));
		}
		virtual booster::aio::io_service &get_io_service()
		{
			return socket_.get_io_service();
		}
		virtual bool keep_alive()
		{
			return false;
		}

		virtual void async_write_eof(handler const &h)
		{
			booster::system::error_code e;
			socket_.shutdown(io::socket::shut_wr,e);
			socket_.get_io_service().post(boost::bind(h,booster::system::error_code()));
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
		size_t start_,end_,sep_;
		booster::shared_ptr<scgi> self()
		{
			return booster::static_pointer_cast<scgi>(shared_from_this());
		}
		friend class socket_acceptor<scgi>;
		io::socket socket_;
		std::vector<char> buffer_;
		std::map<std::string,std::string> env_;
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


