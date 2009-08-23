#ifndef CPPCMS_AIO_TIMER_H
#define CPPCMS_AIO_TIMER_H

#include "defs.h"
#include "noncopyable.h"
#include "callback1.h"
#include "hold_ptr.h"

namespace cppcms {
	class service;
	namespace aio {
		class CPPCMS_API timer : public util::noncopyable {
		public:
			timer(service &srv);
			~timer();

			typedef util::callback1<bool> handler;
			// true if was error or cancelation
			void async_wait(handler const &h);
			void cancel();

			void expires_from_now(int seconds);
			void expires_from_now(int seconds,int milliseconds);
			void expires_at(time_t at);
		private:
			struct data;
			util::hold_ptr<data> d;
		};
	}
}



#endif
