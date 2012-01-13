///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_CGI_ACCEPTOR_H
#define CPPCMS_CGI_ACCEPTOR_H

#include "cgi_api.h"
#include <cppcms/service.h>
#include "service_impl.h"
#include <cppcms/http_context.h>

#include <cppcms/config.h>

#include <booster/aio/socket.h>
#include <booster/aio/endpoint.h>

#ifndef CPPCMS_WIN32
#include <unistd.h>
#endif

namespace io = booster::aio;

namespace cppcms {
namespace impl {
	namespace cgi {

		template<typename API>
		struct server_api_factory {
			API *operator()(cppcms::service &srv) const
			{
				return new API(srv);
			}
		};

		template<class ServerAPI,class Factory = server_api_factory<ServerAPI> >
		class socket_acceptor : public acceptor {
		public:
			socket_acceptor(cppcms::service &srv,std::string ip,int port,int backlog) :
				srv_(srv),
				acceptor_(srv_.get_io_service()),
				stopped_(false),
				tcp_(true)
			{
				io::endpoint ep(ip,port);
				acceptor_.open(ep.family());
				acceptor_.set_option(io::basic_socket::reuse_address,true);
				acceptor_.bind(ep);
				acceptor_.listen(backlog);
			}

			void factory(Factory const &f)
			{
				factory_ = f;
			}

#if !defined(CPPCMS_WIN32)
			socket_acceptor(cppcms::service &srv,int backlog) :
				srv_(srv),
				acceptor_(srv_.get_io_service()),
				stopped_(false),
				tcp_(false)
			{
				acceptor_.attach(0);
				acceptor_.listen(backlog);
			}

			socket_acceptor(cppcms::service &srv,std::string path,int backlog) :
				srv_(srv),
				acceptor_(srv_.get_io_service()),
				stopped_(false),
				tcp_(false)
			{
				io::endpoint ep(path);
				acceptor_.open(io::pf_unix);
				acceptor_.set_option(io::basic_socket::reuse_address,true);
				::unlink(path.c_str());
				acceptor_.bind(ep);
				acceptor_.listen(backlog);
			}
#endif

			struct accept_binder {
				socket_acceptor *self;
				accept_binder(socket_acceptor *s=0) : self(s) {}
				void operator()(booster::system::error_code const &e)
				{
					self->on_accept(e);
				}
			};

			#ifndef CPPCMS_WIN32
			virtual booster::shared_ptr< ::cppcms::http::context> accept(int fd)
			{
				booster::shared_ptr<ServerAPI> api;
				try {
					api.reset(factory_(srv_));
					api->socket_.assign(fd);
					fd=-1;
				}
				catch(...) {
					::close(fd);
					throw;
				}
				if(tcp_)
					api->socket_.set_option(io::basic_socket::tcp_no_delay,true);
				booster::shared_ptr< ::cppcms::http::context> cnt(new ::cppcms::http::context(api));
				return cnt;
			}
			#endif
			virtual booster::aio::acceptor &socket() 
			{
				return acceptor_;
			}
			virtual void async_accept()
			{
				if(stopped_)
					return;
				booster::shared_ptr<ServerAPI> api(factory_(srv_));
				api_=api;
				asio_socket_ = &api->socket_;
				acceptor_.async_accept(*asio_socket_,accept_binder(this));
			}

			virtual void stop()
			{
				stopped_=true;
				acceptor_.cancel();
			}

		private:
			void on_accept(booster::system::error_code const &e)
			{
				if(!e) {
					if(tcp_)
						asio_socket_->set_option(io::basic_socket::tcp_no_delay,true);
					booster::shared_ptr< ::cppcms::http::context> cnt(new ::cppcms::http::context(api_));
					api_.reset();
					cnt->run();	
				}
				async_accept();
			}

			cppcms::service &srv_;
			booster::shared_ptr<connection> api_;
			booster::aio::stream_socket *asio_socket_;
			booster::aio::acceptor acceptor_;
			bool stopped_;
			bool tcp_;
			Factory factory_;
		};


	} // cgi
} // impl
} // cppcms


#endif
