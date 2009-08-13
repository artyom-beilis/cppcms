#ifndef CPPCMS_IMPL_SCGI_API_H
#define CPPCMS_IMPL_SCGI_API_H

#include "cgi_api.h"
#include "asio_config.h"
#include "service.h"
#include "service_impl.h"
#include "cppcms_error_category.h"
#include <iostream>

namespace cppcms {
namespace impl {
namespace cgi {
	class http : public connection {
	public:
		http(cppcms::service &srv) :
			connection(srv),
			socket_(srv.impl().get_io_service())
		{
		}
		virtual void async_read_headers(handler const &h)
		{
			body_.reserve(8192);
			body_.resize(8192,0);
			body_ptr_=0;
			socket_.async_read_some(boost::asio::buffer(body_),
				boost::bind(	&http::some_headers_data_read,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred,
						h));
		}

		void some_headers_data_read(boost::system::error_code const &e,size_t n,handler const &h)
		{
			if(e) { h(e); return; }
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
		virtual void async_read_some(void *p,size_t s,io_handler const &h)
		{
			if(!body_.empty()) {
				if(body_.size() - body_ptr_ > s) {
					s=body_.size() -  body_ptr_;
				}
				memcpy(p,&body_[body_ptr_],s);
				body_ptr_+=s;
				if(body_ptr_==body_.size()) {
					body_.clean();
					body_ptr_=0;
				}
				socket_.get_io_service().post(boost::bind(h,boost::system::error_code(),s));
				return;
			}
			socket_.async_read_some(boost::asio::buffer(p,s),h);
		}
		virtual void async_write_some(void const *p,size_t s,io_handler const &h)
		{
			socket_.async_write_some(boost::asio::buffer(p,s),h);
		}
		virtual size_t write_some(void const *buffer,size_t n)
		{
			return socket_.write_some(boost::asio::buffer(buffer,n));
		}
		virtual boost::asio::io_service &get_io_service()
		{
			return socket_.get_io_service();
		}
		virtual bool keep_alive()
		{
			return false;
		}

		virtual void close()
		{
			boost::system::error_code e;
			socket_.shutdown(boost::asio::basic_stream_socket<Proto>::shutdown_both,e);
			socket_.close(e);
		}

	private:

		class parser {
			enum {
				idle,
			} state_;

			inline int getc()
			{
				if(body_ptr_ < body_.size()) {
					return body_[body_ptr++];
				}
				else {
					body_.clean();
					body_ptr_=0;
					return -1;
				}
			}
			inline void ungetc(int c)
			{
				if(body_ptr_ > 0) {
					body_ptr--;
					body_[body_ptr_]=c'
				}
				else {
					body_.insert(body_.begin(),c);
				}
			}
		public:
			enum { more_data, got_header, end_of_headers , error_observerd };
			int step()
			{
				int c=getc();
				if(c<0)
					return more_data;

				switch(state_)  {
				case idle:
					switch(c) {
					case '\r':
						state_=last_lf_exptected;
						break;
					case '"':
						state_=quote_expected;
						break;
					default:
						state_=input_observed;
					}
					break;
				case last_lf_exptected:
					if(c!='\n') return error_observerd;
					header_.clear();
					return end_of_headers;
				case lf_exptected:
					if(c!='\n') return error_observerd;
					state_=space_or_other_exptected;
					break;
				case space_or_other_exptected:
					if(c==' ' || c=='\t') {
						state_=input_observed;
						break;
					}
					ungetc(c);
					header_.resize(header_.size()-2);
					state_=idle;
					return got_header;					
				case input_observed:
					switch(c) {
					case '\r':
						state_=lf_exptected;
						break;
					case '"':
						state_=quote_expected;
						break;
					default:
						state_=input_observed;
					}
					break;
				case quote_expected:
					switch(c) {
					case '"':
						state_=input_observed;
						break;
					case '\\':
						state_=pass_quote_exptected;
						break;
					}
					break;
				case pass_quote_exptected:
					if(c < 0 || c >=127)
						return error_observerd;
					state_=quote_expected;
					break;
				}
				header_+=char(c);
			}
		};

		boost::shared_ptr<scgi<Proto> > shared_from_this()
		{
			return boost::static_pointer_cast<scgi<Proto> >(connection::shared_from_this());
		}
		friend class socket_acceptor<boost::asio::ip::tcp,http>;
		boost::asio::ip::tcp socket_;
		std::vector<char> body_;
		std::map<std::string,std::string> env_;
	};

	typedef tcp_socket_acceptor<http>    http_acceptor;


} // cgi
} // impl
} // cppcms


#endif
