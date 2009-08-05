#ifndef CPPCMS_SERVICE_H
#define CPPCMS_SERVICE_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"


namespace cppcms {
	namespace impl {
		class service;
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
		void shutdown();

		cppcms::applications_pool &applications_pool();
		cppcms::thread_pool &thread_pool();
		cppcms::cppcms_config const &settings();

		cppcms::impl::service &impl();
	private:
		void stop();
		void start_acceptor();
		void setup_exit_handling();
		int threads_no();
		int procs_no();
		bool prefork();
		util::hold_ptr<impl::service> impl_;
	};

} //




#endif
