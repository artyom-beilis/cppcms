#ifndef CPPCMS_CGI_ACCEPTOR_H
#define CPPCMS_CGI_ACCEPTOR_H

namespace cppcms {
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


	} // cgi
} // cppcms


#endif
