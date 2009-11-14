#ifndef CPPCMS_CGI_ACCEPTOR_H
#define CPPCMS_CGI_ACCEPTOR_H

#include "asio_config.h"
#include "cgi_api.h"
#include "service.h"
#include "service_impl.h"
#include "http_context.h"

#include <boost/bind.hpp>

namespace cppcms {
namespace impl {
	namespace cgi {

		template<typename Proto,class ServerAPI>
		class socket_acceptor : public acceptor {
		public:
			socket_acceptor(cppcms::service &srv) :
				srv_(srv),
				acceptor_(srv_.impl().get_io_service()),
				stopped_(false)
			{
			}
			virtual void async_accept()
			{
				if(stopped_)
					return;
				ServerAPI *api=new ServerAPI(srv_);
				api_=api;
				asio_socket_ = &api->socket_;
				acceptor_.async_accept(
					*asio_socket_,
					boost::bind(	&socket_acceptor::on_accept,
							this,
							boost::asio::placeholders::error));
			}
			virtual void stop()
			{
				stopped_=true;
				boost::system::error_code e;
				#ifdef CPPCMS_WIN32
				acceptor_.close(e);
				#else
				acceptor_.cancel(e);
				#endif	
			}
		private:
			void on_accept(boost::system::error_code const &e)
			{
				if(!e) {
					set_options(asio_socket_);
					intrusive_ptr<::cppcms::http::context> cnt(new ::cppcms::http::context(api_));
					api_=0;
					cnt->run();	
				}
				async_accept();
			}
			void set_options(boost::asio::ip::tcp::socket *socket)
			{
				socket->set_option(boost::asio::ip::tcp::no_delay(true));
			}
#if !defined(CPPCMS_WIN32)
			void set_options(boost::asio::local::stream_protocol::socket *socket)
			{
				// nothing;
			}
#endif
		protected:

			cppcms::service &srv_;
			intrusive_ptr<connection> api_;
			boost::asio::basic_stream_socket<Proto> *asio_socket_;
			boost::asio::basic_socket_acceptor<Proto> acceptor_;
			bool stopped_;
		};

#if !defined(CPPCMS_WIN32)
		template<typename API>
		class unix_socket_acceptor : public socket_acceptor<boost::asio::local::stream_protocol,API>
		{
		public:
			// Listen on fd provided as stdin
			unix_socket_acceptor(cppcms::service &srv,int backlog)  :
				socket_acceptor<boost::asio::local::stream_protocol,API>(srv)
			{
				boost::asio::local::stream_protocol::acceptor 
					&acceptor=socket_acceptor<boost::asio::local::stream_protocol,API>::acceptor_;
				acceptor.assign(boost::asio::local::stream_protocol(),0);
				acceptor.listen(backlog);
			}
			unix_socket_acceptor(cppcms::service &srv,std::string const &path,int backlog) :
				socket_acceptor<boost::asio::local::stream_protocol,API>(srv)
			{
				boost::asio::local::stream_protocol::endpoint ep(path);
				boost::asio::local::stream_protocol::acceptor 
					&acceptor=socket_acceptor<boost::asio::local::stream_protocol,API>::acceptor_;

				acceptor.open(ep.protocol());
				acceptor.set_option(boost::asio::local::stream_protocol::acceptor::reuse_address(true));
				::unlink(path.c_str());
				acceptor.bind(boost::asio::local::stream_protocol::endpoint(path));
				acceptor.listen(backlog);
			}
		};
#endif
		template<typename API>
		class tcp_socket_acceptor : public socket_acceptor<boost::asio::ip::tcp,API>
		{
		public:
			tcp_socket_acceptor(cppcms::service &srv,std::string const &ip,int port,int backlog) :
				socket_acceptor<boost::asio::ip::tcp,API>(srv)
			{
				boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(ip),port);
				boost::asio::ip::tcp::acceptor 
					&acceptor=socket_acceptor<boost::asio::ip::tcp,API>::acceptor_;

				acceptor.open(ep.protocol());
				acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
				acceptor.bind(ep);
				acceptor.listen(backlog);
			}
		};


	} // cgi
} // impl
} // cppcms


#endif
