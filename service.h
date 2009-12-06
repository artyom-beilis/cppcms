#ifndef CPPCMS_SERVICE_H
#define CPPCMS_SERVICE_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"
#include "callback0.h"
#include <locale>

namespace boost { namespace locale { class generator; }}

namespace cppcms {
	namespace locale { using namespace boost::locale; }
	namespace impl {
		class service;
	}

	class applications_pool;
	class thread_pool;
	class views_pool;
	namespace json {
		class value;
	}

	class CPPCMS_API service : public util::noncopyable
	{
	public:
		service(json::value const &v);
		service(int argc,char *argv[]);
		~service();

		void run();
		void shutdown();

		cppcms::applications_pool &applications_pool();
		cppcms::thread_pool &thread_pool();
		json::value const &settings();
		cppcms::views_pool &views_pool();

		locale::generator const &generator();
		std::locale locale();
		std::locale locale(std::string const &name);

		cppcms::impl::service &impl();

		void post(util::callback0 const &handler);
		
		int threads_no();
		int procs_no();
	private:
		void setup();
		void load_settings(int argc,char *argv[]);
		void stop();
		void start_acceptor();
		void setup_exit_handling();
		bool prefork();
		util::hold_ptr<impl::service> impl_;
	};

} //




#endif
