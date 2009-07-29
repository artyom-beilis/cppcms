#ifndef CPPCMS_CGI_ACCEPTOR_H
#define CPPCMS_CGI_ACCEPTOR_H

namespace cppcms {
namespace impl {
	namespace cgi {

		template<typename Proto,class ServerAPI>
		class socket_acceptor : public acceptor {
		public:
			scgi_acceptor(boost::asio::io_service &srv) : 
				srv_(srv),
				acceptor_(srv_)
			{
			}
			virtual void async_accept()
			{
				ServerAPI *api=new ServerAPI(service());
				api_.reset(api);
				asio_socket_ = &api->socket_;
				acceptor_.async_accept(
					*asio_socket_,
					boost::bind(	&scgi_acceptor::on_accept,
							this,
							boost::asio::placeholder::error));
			}
		private:
			void on_accept(boost::system::error_code const &e)
			{
				if(!e) {
					set_options(asio_socket_);
					api_->on_accepted();
					return;
				}
				async_accept();
			}
			void set_options(boost::asio:ip::tcp::socket *socket)
			{
				socket->set_option(boost::asio::ip::tcp::no_delay(true));
			}

			void set_options(boost::asio::local::socket *socket)
			{
				// nothing;
			}


			boost::shared_ptr<connection> api_;
			boost::asio::basic_stream_socket<Proto> asio_socket_;
			boost::asio::basic_socket_acceptor<Proto> acceptor_;
		};

		template<typename API>
		class unix_socket_acceptor : public socket_acceptor<boost::asio::local::stream_protocol,API> 
		{
		public:
			unix_socket_acceptor(service &srv,std::string const &path,int backlog) :
				socket_acceptor(srv)
			{
				acceptor_.open();
				acceptor_.bind(boost::asio::local::endpoint(path));
				acceptor_.listen(backlog);
			}
		}
		template<typename API>
		class tcp_socket_acceptor : public socket_acceptor<boost::asio::ip::tcp,API> 
		{
		public:
			tcp_socket_acceptor(service &srv,std::string const &ip,int port,int backlog) :
				socket_acceptor(srv)
			{
				acceptor_.open();
				boost::asio::ip::endpoint ep(boost::asio::ip::address::from_string(ip),port);
				acceptor_.bind(ep);
				acceptor_.listen(backlog);
			}
		}


	} // cgi
} // impl
} // cppcms


#endif
