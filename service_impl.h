#ifndef CPPCMS_SERVICE_IMPL_H
#define CPPCMS_SERVICE_IMPL_H

#include "asio_config.h"
#include <memory>

namespace cppcms {
class service;
class cppcms_config;
class applications_pool;
class thread_pool;

namespace impl {
	namespace cgi {
		class acceptor;
	}

	class service : public util::noncopyable {
	public:
		service();
		~service();
		boost::asio::io_service &get_io_service()
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
#ifdef CPPCMS_WIN32
		boost::asio::ip::tcp::socket sig_,breaker_;
#else
		boost::asio::local::stream_protocol::socket sig_,breaker_;
#endif

	};


}
} // cppcms



#endif
