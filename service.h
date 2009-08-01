#ifndef CPPCMS_SERVICE_H
#define CPPCMS_SERVICE_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"


namespace cppcms {
	namespace impl {
		class service_impl;
	}

	class applications_pool;
	class thread_pool;
	class cppcms_config;

	class CPPCMS_API service : public util::noncopyable
	{
	public:
		service(int argc,char *argv[]);
		~service();

		void run();

		cppcms::applications_pool &appications_pool();
		cppcms::thread_pool &thread_pool();
		cppcms::cppcms_config const &settings();

		cppcms::impl::service_impl &impl();
	private:
		void start_acceptor();
		int threads_no();
		util::hold_ptr<impl::service_impl> impl_;
	};

} //




#endif
