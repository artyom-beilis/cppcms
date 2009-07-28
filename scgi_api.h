#ifndef CPPCMS_IMPL_SCGI_API_H
#define CPPCMS_IMPL_SCGI_API_H

namespace cppcms {
namespace impl {
namespace cgi {
	template<typename Proto,typename API> class socket_acceptor;
	template<typename Proto>
	class scgi : public api {

		scgi(service &srv) :
			api(srv),
			start_(0),
			end_(0),
			socket_(srv.impl().io_service())
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
			size_t size=buffer_.size();
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
			std::vector<char>::const_iterator p=buffer_.begin() + sep_ + 1;
			while(p+1 < buffer_.end()) {
				// TODO
			}
		}

		// should be called only after headers are read
		virtual std::string getenv(std::string const &key) = 0;
		virtual void async_read_some_post_data(void *,size_t n,io_handler const &h) = 0;
		virtual bool keep_alive() = 0;

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
};


} // cgi
} // impl
} // cppcms


#endif
