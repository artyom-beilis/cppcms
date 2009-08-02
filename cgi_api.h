#ifndef CPPCMS_IMPL_CGI_API_H
#define CPPCMS_IMPL_CGI_API_H

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/system/error_code.hpp>


namespace cppcms {
namespace impl {
namespace cgi {

	typedef boost::function<void(boost::system::error_code const &e)> handler;
	typedef boost::function<void(boost::system::error_code const &e,size_t)> io_handler;

	class acceptor : public util::noncopyable {
	public:
		virtual void async_accept() = 0;
		virtual void stop() = 0;
		virtual ~acceptor(){}
	};

	class connection :
		public boost::enable_shared_from_this<connection>,
		public util::noncopyable
	{
	public:
		void on_accepted();
		connection(cppcms::service &srv);
		virtual ~connection();

		// These are abstract member function that should be implemented by
		// actual protocol like FCGI, SCGI, HTTP or CGI

		virtual void async_read_headers(handler const &h) = 0;
		virtual std::string getenv(std::string const &key) = 0;
		virtual bool keep_alive() = 0;

		// Concept implementation headers		
		
		virtual void async_read_some(boost::asio::mutable_buffers_1 const &buf,io_handler const &h) = 0;
		virtual void async_write_some(boost::asio::const_buffers_1 const &buf,io_handler const &h) = 0;
		virtual boost::asio::io_service &io_service() = 0;

		// end of abstract functions
	private:
		long long check_valid_content_length();
		void on_headers_read(boost::system::error_code const &e);
		void on_content_recieved(boost::system::error_code const &e,size_t);
		void on_post_data_read();
		void setup_application();
		void dispach(std::string const &path,bool thread);
		void on_error(std::string const &msg);
		void on_ready_response();
		void write_response();
		void on_response_complete();


	};


} // cgi
} // impl
} // cppcms

#endif
