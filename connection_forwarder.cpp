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
#include "connection_forwarder.h"
#include "http_context.h"
#include "http_request.h"
#include "http_response.h"
#include "service.h"
#include "service_impl.h"
#include "url_dispatcher.h"
#include "cgi_api.h"
#include <booster/aio/socket.h>
#include <booster/aio/io_service.h>
#include <booster/aio/buffer.h>
#include <booster/aio/endpoint.h>

#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#   include <boost/enable_shared_from_this.hpp>
#   include <boost/format.hpp>
#   if defined(CPPCMS_WIN32) && _WIN32_WINNT <= 0x0501 && !defined(BOOST_ASIO_DISABLE_IOCP)
#   define NO_CANCELIO
#   endif
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
#   include <cppcms_boost/enable_shared_from_this.hpp>
#   include <cppcms_boost/format.hpp>
    namespace boost = cppcms_boost;
#   if defined(CPPCMS_WIN32) && _WIN32_WINNT <= 0x0501 && !defined(CPPCMS_BOOST_ASIO_DISABLE_IOCP)
#   define NO_CANCELIO
#   endif
#endif

namespace io = booster::aio;

namespace cppcms {
	namespace impl {
		class tcp_pipe : public boost::enable_shared_from_this<tcp_pipe> {
		public:
			tcp_pipe(intrusive_ptr<http::context> connection,std::string const &ip,int port) :
				connection_(connection),
				ip_(ip),
				port_(port),
				socket_(connection_->service().impl().get_io_service())
			{
			}
			void async_send_receive(std::string &data)
			{
				data_.swap(data);
				io::endpoint ep(ip_,port_);
				socket_.open(ep.family(),io::sock_stream);
				socket_.async_connect(ep,
					boost::bind(
						&tcp_pipe::on_connected,
						shared_from_this(),_1));
			}
		private:

			void on_connected(booster::system::error_code e) 
			{
				if(e) {
					connection_->response().make_error_response(500);
					connection_->async_complete_response();
					return;
				}
				socket_.async_write(
					io::buffer(data_),
					boost::bind(&tcp_pipe::on_written,shared_from_this(),_1));
			}

			void on_written(booster::system::error_code const &e)
			{
				if(e) {
					connection_->response().make_error_response(500);
					connection_->async_complete_response();
					return;
				}

				
				connection_->async_on_peer_reset(boost::bind(&tcp_pipe::on_peer_close,shared_from_this()));
				connection_->response().io_mode(http::response::asynchronous_raw);
				input_.resize(4096);
				socket_.async_read_some(io::buffer(input_),
					boost::bind(&tcp_pipe::on_read,shared_from_this(),
						_1,
						_2));

			}

			void on_peer_close()
			{
				booster::system::error_code ec;
				socket_.cancel();
				socket_.shutdown(io::socket::shut_rdwr,ec);
				socket_.close(ec);
				return;
			}

			void on_read(booster::system::error_code const &e,size_t n)
			{
				if(n > 0) {
					connection_->response().out().write(&input_.front(),n);
				}
				if(e) {
					connection_->async_complete_response();
				}
				else {
					socket_.async_read_some(io::buffer(input_),
						boost::bind(&tcp_pipe::on_read,shared_from_this(),
							_1,
							_2));
				}
			}

			intrusive_ptr<http::context> connection_;
			std::string ip_;
			int port_;
			std::string data_;
			io::socket socket_;
			std::vector<char> input_;
		};
	} // impl

	struct connection_forwarder::data {};

	connection_forwarder::connection_forwarder(cppcms::service &srv,std::string const &ip,int port) :
		application(srv),
		ip_(ip),
		port_(port)
	{
	}
	connection_forwarder::~connection_forwarder()
	{
	}
	void connection_forwarder::main(std::string unused)
	{
		intrusive_ptr<http::context> con = release_context();
		std::string env_str;
		env_str.reserve(1000);

		std::pair<void *,size_t> post = con->request().raw_post_data();
		std::map<std::string,std::string> const &env = con->connection().getenv();
		std::map<std::string,std::string>::const_iterator cl;

		cl=env.find("CONTENT_LENGTH");
		if(cl!=env.end()) {
			env_str.append(cl->first.c_str(),cl->first.size()+1);
			env_str.append(cl->second.c_str(),cl->second.size()+1);
		}
		else {
			env_str.append("CONTENT_LENGTH");
			env_str.append("\0" "0",3);
		}

		for(std::map<std::string,std::string>::const_iterator p=env.begin();p!=env.end();++p) {
			if(p==cl)
				continue;
			env_str.append(p->first.c_str(),p->first.size()+1);
			env_str.append(p->second.c_str(),p->second.size()+1);
		}
		std::string header=(boost::format("%1%:",std::locale::classic()) % env_str.size()).str();
		header.reserve(header.size()+env_str.size()+post.second);
		header+=env_str;
		header+=',';
		header.append(reinterpret_cast<char *>(post.first),post.second);
		
		boost::shared_ptr<impl::tcp_pipe> pipe(new impl::tcp_pipe(con,ip_,port_));
		pipe->async_send_receive(header);
	}
};
