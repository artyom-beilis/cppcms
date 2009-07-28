#ifndef CPPCMS_SCGI_API_H
#define CPPCMS_SCGI_API_H

namespace cppcms {
	namespace cgi {
		template<typename Proto,typename API> class socket_acceptor;
		template<typename Proto>
		class scgi : public api {

			scgi(service &srv) : 
				api(srv)
			{
			}
			virtual void async_read_headers(handler const &h)
			{
				buffer_.resize(1024);
				boost::asio::async_read(
					*socket_
					boost::asio::buffer(buffer_),
					boost::asio::transfer_at_least(16),
					boost::bind(
						&scgi::on_read,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));

			}
			// should be called only after headers are read
			virtual std::string getenv(std::string const &key) = 0;
			virtual void async_read_some_post_data(void *,size_t n,io_handler const &h) = 0;
			virtual bool keep_alive() = 0; 

		private:
			boost::shared_ptr<scgi<Proto> > shared_from_this()
			{
				return boost::static_pointer_cast<scgi<Proto> >(api::shared_from_this());
			}
			friend class socket_acceptor<Proto,scgi<Proto> >;
			boost::asio::basic_stream_socket<Proto> socket_;
			std::vector<char> buffer_;
		};
	};
} // cppcms


#endif
