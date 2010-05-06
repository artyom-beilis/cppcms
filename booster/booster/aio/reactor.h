#ifndef BOOSTER_AIO_REACTOR_H
#define BOOSTER_AIO_REACTOR_H
#include <booster/config.h>
#include <booster/aio/types.h>
#include <memory>
#include <string>
namespace booster {
namespace aio {

	class reactor_impl;

	class BOOSTER_API reactor {
		reactor(reactor const &);
		void operator=(reactor const &);
	public:
		static const int in   = 1 << 0;
		static const int out  = 1 << 1;
		static const int err  = 1 << 2;

		static const int use_default	= 0;
		static const int use_select	= 1;
		static const int use_poll	= 2;
		static const int use_epoll	= 3;
		static const int use_dev_poll	= 4;
		static const int use_kqueue	= 5;
		static const int use_max	= use_kqueue;

		struct event {
			native_type fd;
			int events;
		};


		reactor(int hint = use_default);
		~reactor();

		void select(native_type fd,int flags);
		void select(native_type fd,int flags,system::error_code &e);
		void remove(native_type fd)
		{
			select(fd,0);
		}
		void remove(native_type fd,system::error_code &e)
		{
			select(fd,0,e);
		}
		
		int poll(event *events,int n,int timeout);
		int poll(event *events,int n,int timeout,system::error_code &e);

		std::string name() const;
	private:
		std::auto_ptr<reactor_impl> impl_;
	};
} // aio
} // booster


#endif
