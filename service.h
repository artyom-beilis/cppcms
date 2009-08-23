#ifndef CPPCMS_SERVICE_H
#define CPPCMS_SERVICE_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"
#include "callback0.h"



namespace cppcms {
	namespace impl {
		class service;
	}

	class applications_pool;
	class thread_pool;
	namespace locale {
		class pool;
	}
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
		cppcms::locale::pool const &locale_pool();

		cppcms::impl::service &impl();

		void post(util::callback0 const &handler);
		
		int threads_no();
		int procs_no();
	private:
		void stop();
		void start_acceptor();
		void setup_exit_handling();
		bool prefork();
		util::hold_ptr<impl::service> impl_;
	};

} //




#endif
