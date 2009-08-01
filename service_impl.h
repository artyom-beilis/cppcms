#ifndef CPPCMS_SERVICE_IMPL_H
#define CPPCMS_SERVICE_IMPL_H

#include <boost/asio.hpp>

namespace cppcms {
class service;
class cppcms_config;
class applications_pool;
class thread_pool;

namespace impl {

	class service : public util::noncopyable {
	public:
		service();
		~service();
		boost::asio::io_service &io_service()
		{
			return io_service_;
		}


	private:
		friend class cppcms::service;

		std::auto_ptr<cgi::acceptor> acceptor_;
		std::auto_ptr<cppcms_config> settings_;
		std::auto_ptr<applications_pool> applications_pool_;
		std::auto_ptr<thread_pool> thread_pool_;

		boost::asio::io_service io_service_;
#ifndef _WIN32
		boost::asio::local::stream_protocol::socket sig_,breaker_;
#endif

	};


}
} // cppcms



#endif