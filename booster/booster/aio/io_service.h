#ifndef BOOSTER_AIO_IO_SERVICE_H
#define BOOSTER_AIO_IO_SERVICE_H

#include <booster/config.h>
#include <booster/aio/aio_config.h>
#include <booster/aio/types.h>
#include <booster/thread.h>
#include <booster/system_error.h>
#include <booster/aio/aio_category.h>
#include <booster/function.h>
#include <booster/noncopyable.h>
#include <string>
#include <memory>

namespace booster {
class ptime;
namespace aio {

	typedef function<void(system::error_code const &e,int fd)> accept_handler;
	class event_loop_impl;
	class BOOSTER_API io_service : public noncopyable {
	public:

		static const int in = 	1 << 0;
		static const int out =	1 << 1;
		
		io_service(int reactor_type);
		io_service();
		~io_service();

		#ifndef BOOSTER_WIN32
		void set_prefork_acceptor(native_type fd,accept_handler const &h);
		void cancel_prefork_acceptor(native_type fd);
		void enable_prefork();
		#endif

		void set_io_event(native_type fd,int event,event_handler const &h);
		void cancel_io_events(native_type fd);

		int set_timer_event(ptime const &point,event_handler const &h);
		void cancel_timer_event(int event_id);

		void run();
		void run(system::error_code &e);
		void reset();
		void stop();
		void post(handler const &h);

		std::string reactor_name();
	private:
		struct data;
		hold_ptr<data> d;
		std::auto_ptr<event_loop_impl> impl_;	
	};

} // aio
} // booster

#endif
