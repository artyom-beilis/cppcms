///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/forwarder.h>
#include <cppcms/http_context.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/mount_point.h>
#include <cppcms/service.h>
#include "service_impl.h"
#include <cppcms/url_dispatcher.h>
#include "cgi_api.h"
#include <booster/aio/socket.h>
#include <booster/aio/io_service.h>
#include <booster/aio/buffer.h>
#include <booster/aio/endpoint.h>

#include <booster/shared_ptr.h>
#include <booster/enable_shared_from_this.h>
#include <scgi_header.h>

#include "todec.h"

#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#   if defined(CPPCMS_WIN32) && _WIN32_WINNT <= 0x0501 && !defined(BOOST_ASIO_DISABLE_IOCP)
#   define NO_CANCELIO
#   endif
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#   if defined(CPPCMS_WIN32) && _WIN32_WINNT <= 0x0501 && !defined(CPPCMS_BOOST_ASIO_DISABLE_IOCP)
#   define NO_CANCELIO
#   endif
#endif

namespace io = booster::aio;

namespace cppcms {
	namespace impl {
		class tcp_pipe : public booster::enable_shared_from_this<tcp_pipe> {
		public:
			tcp_pipe(booster::shared_ptr<http::context> connection,std::string const &ip,int port) :
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
				socket_.open(ep.family());
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
				socket_.shutdown(io::stream_socket::shut_rdwr,ec);
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

			booster::shared_ptr<http::context> connection_;
			std::string ip_;
			int port_;
			std::string data_;
			io::stream_socket socket_;
			std::vector<char> input_;
		};

		std::string make_scgi_header(std::map<std::string,std::string> const &env,size_t addon_size)
		{
			std::string env_str;
			env_str.reserve(1000);

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
			std::string header=todec_string(env_str.size());
			header+=':';
			header.reserve(header.size()+env_str.size()+addon_size);
			header+=env_str;
			header+=',';
			return header;
		}

	} // impl

	
	void forward_connection(booster::shared_ptr<http::context> con,std::string const &ip,int port)
	{

		std::map<std::string,std::string> const &env = con->connection().getenv();
		std::pair<void *,size_t> post = con->request().raw_post_data();
		std::string header = impl::make_scgi_header(env,post.second);
		header.append(reinterpret_cast<char *>(post.first),post.second);
		
		booster::shared_ptr<impl::tcp_pipe> pipe(new impl::tcp_pipe(con,ip,port));
		pipe->async_send_receive(header);
	}



	struct forwarder::_data {};
	forwarder::forwarder()
	{
	}
	forwarder::~forwarder()
	{
	}
	
	void forwarder::add_forwarding_rule(booster::shared_ptr<mount_point> p,std::string const &ip,int port)
	{
		booster::unique_lock<booster::shared_mutex> lock(mutex_);
		rules_[p]=address_type(ip,port);
	}
	void forwarder::remove_forwarding_rule(booster::shared_ptr<mount_point> p)
	{
		booster::unique_lock<booster::shared_mutex> lock(mutex_);
		rules_.erase(p);
	}
	forwarder::address_type forwarder::check_forwading_rules(char const *h,char const *s,char const *p)
	{
		booster::shared_lock<booster::shared_mutex> lock(mutex_);
		for(rules_type::const_iterator it=rules_.begin();it!=rules_.end();++it) {
			if(it->first->match(h,s,p).first)
				return it->second;
		}
		return address_type(std::string(),0);
	}
	forwarder::address_type forwarder::check_forwading_rules(std::string const &h,std::string const &s,std::string const &p)
	{
		booster::shared_lock<booster::shared_mutex> lock(mutex_);
		for(rules_type::const_iterator it=rules_.begin();it!=rules_.end();++it) {
			if(it->first->match(h,s,p).first)
				return it->second;
		}
		return address_type(std::string(),0);
	}


};
