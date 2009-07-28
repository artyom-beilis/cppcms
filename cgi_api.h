#ifndef CPPCMS_CGI_API_H
#define CPPCMS_CGI_API_H

#ifdef CPPCMS_PRIV_BOOST_H
#include <cppcms_boost/enable_shared_from_this.hpp>
#include <cppcms_boost/function.hpp>
#include <cppcms_boost/system/error_code.hpp>
namespace boost = cppcms_boost;
#else // general boost
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/system/error_code.hpp>
#endif


namespace cppcms {
	namespace cgi { 

	typedef boost::function<void(boost::system::error_code const &e)> handler;
	typedef boost::function<void(boost::system::error_code const &e,size_t)> io_handler;

	class acceptor : public util::noncopyable {
	public:
		virtual void async_accept() = 0;
		virtual ~acceptor(){}
	};

	class connection :
		public boost::shared_from_this<connection>, 
		public util::noncopyable
	{
	public:
		void on_accepted();
		virtual ~connection();
		
		// These are abstract member function that should be implemented by
		// actual protocol like FCGI, SCGI, HTTP or CGI
		
		virtual void async_read_headers(handler const &h) = 0;
		// should be called only after headers are read
		virtual std::string getenv(std::string const &key) = 0;
		virtual void async_read_some_post_data(void *,size_t n,io_handler const &h) = 0;
		virtual bool keep_alive() = 0; 

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


	}


};

#endif
