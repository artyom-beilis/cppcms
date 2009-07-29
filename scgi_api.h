#ifndef CPPCMS_IMPL_SCGI_API_H
#define CPPCMS_IMPL_SCGI_API_H

namespace cppcms {
namespace impl {
namespace cgi {
	template<typename Proto,typename API> class socket_acceptor;
	template<typename Proto>
	class scgi : public api {
	public:
		scgi(service &srv) :
			api(srv),
			start_(0),
			end_(0),
			socket_(srv.impl().io_service())
		{
		}
		virtual void async_read_headers(handler const &h)
		{
			buffer_.resize(16);
			boost::asio::async_read(
				*socket_
				boost::asio::buffer(buffer_),
				boost::bind(
					&scgi::on_first_read,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					h));

		}

		void on_first_read(boost::system::error_code const &e,size_t n,handler const &h)
		{
			if(e) {
				h(e);
				return;
			}
			sep_=std::find(buffer_.begin(),buffer_.begin()+n,':') - buffer_.begin();
			if(sep_ > 16) {
				h(boost::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}
			buffer_[sep_]=0;
			int len=atoi(&buffer_.front());
			if(len > 16384) {
				h(boost::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}
			size_t size=n;
			buffer_.resize(sep_ + 2 + len); // len of number + ':' + content + ','
			boost::asio::async_read(*socket_,
					boost::asio::buffer(&buffer_[size],buffer_.size() - size),
					boost::bind(	&scgi::on_headers_chunk_read,
							shared_from_this(),
							boost::asio::placeholders::error,
							h));
		}
		void on_headers_chunk_read(boost::system::error_code const &e,handler const &h)
		{
			if(e) { h(e); return; }
			if(buffer_.back()!=',') {
				buffer_.back() = 0;
				// make sure it is NUL terminated
				h(boost::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}
			char const *p=&buffer_[sep_ + 1];
			while(p < &buffer_.back()) {
				std::string key=p;
				p+=key.size();
				if(p>=&buffer.back())
					break;
				std::string value=p;
				p+=value.size();
				env_.insert(std::pair<std::string,std::string>(key,value));
			}
			buffer_.clear();
			h(boost::system::error_code());
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
		virtual void async_read_some(void *p,size_t n,io_handler const &h)
		{
			socket_.async_read_some(boost::asio::buffer(p,n),h);
		}
		virtual bool keep_alive()
		{
			return false;
		}

	private:
		size_t start_,end_;
		boost::shared_ptr<scgi<Proto> > shared_from_this()
		{
			return boost::static_pointer_cast<scgi<Proto> >(api::shared_from_this());
		}
		friend class socket_acceptor<Proto,scgi<Proto> >;
		boost::asio::basic_stream_socket<Proto> socket_;
		std::vector<char> buffer_;
		std::map<std::string,std::string> env_;
	};

	typedef scgi<boost::asio::local::stream_protocol> unix_socket_scgi;
	typedef scgi<boost::asio::ip::tcp> tcp_socket_scgi;
	typedef unix_socket_acceptor<unix_socket_scgi>    unix_socket_scgi_acceptor;
	typedef tcp_socket_acceptor<tcp_socket_scgi>    tcp_socket_scgi_acceptor;

};


} // cgi
} // impl
} // cppcms


#endif
