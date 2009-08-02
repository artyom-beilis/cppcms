#ifndef CPPCMS_IMPL_CGI_API_H
#define CPPCMS_IMPL_CGI_API_H

#include "noncopyable.h"

#include <vector>

#include "asio_config.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/system/error_code.hpp>



namespace cppcms {
	class service;
	class application;
	namespace http {
		class context;
	}


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
		
		cppcms::service &service();


		/****************************************************************************/

		// These are abstract member function that should be implemented by
		// actual protocol like FCGI, SCGI, HTTP or CGI

		virtual void async_read_headers(handler const &h) = 0;
		virtual std::string getenv(std::string const &key) = 0;
		virtual bool keep_alive() = 0;

		// Concept implementation headers		
		
		virtual void async_read_some(void *,size_t,io_handler const &h) = 0;
		virtual void async_write_some(void const *,size_t,io_handler const &h) = 0;
		virtual boost::asio::io_service &get_io_service() = 0;

		/****************************************************************************/

	protected:
		void async_read(void *,size_t,io_handler const &h);
		void async_write(void const *,size_t,io_handler const &h);
	private:
		void load_content(boost::system::error_code const &e);
		void on_content_read(boost::system::error_code const &e);
		void process_request(boost::system::error_code const &e);

		void load_multipart_form_data();
		void make_error_response(int statis,char const *msg="");
		void setup_application();
		void dispatch(bool thread);
		void on_error(std::string const &msg);
		void on_response_ready();
		void write_response();
		void on_response_complete(boost::system::error_code const &e=boost::system::error_code());

		std::auto_ptr<http::context> context_;
		std::vector<char> content_;
		std::auto_ptr<application> application_;

		cppcms::service *service_;

	};


} // cgi
} // impl
} // cppcms

#endif
