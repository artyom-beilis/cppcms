#ifndef CPPCMS_IMPL_PROC_ACCEPTOR_H
#define CPPCMS_IMPL_PROC_ACCEPTOR_H

#include "posix_util.h"
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/thread.hpp>
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/thread.hpp>
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

#include "function.h"
#include "service.h"
#include "service_impl.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace cppcms {
	namespace impl {
		class process_shared_acceptor : public util::noncopyable {
		public:
			typedef function<void(boost::system::error_code const &e)> on_accept_function;

			process_shared_acceptor(cppcms::service &srv);
			~process_shared_acceptor();

			template<typename Socket1,typename Socket2>
			void async_accept(Socket1 acc,Socket2 sock,on_accept_function const &on_accepted)
			{
				int fd = acc.native();
				{
					thread_guard guard(thread_mutex_);
					FD_SET(fd,&wait_set);
					if(fd + 1 > max_1_ )
						max_1_ = fd+1;
					callbacks_[fd]=on_accepted;
					assigners_[fd]=boost::bind(&Socket2::assign,&sock,_1);
				}
				wake();
			}
			void stop()
			{
				stop_ = true;
				wake();
			}
		private:
			void on_fork();
			void wake();
			bool run_one();
			void run();
			
			typedef function<void(int)> assign_function;
			bool stop_;
			int wake_fd_,break_fd_;
			int max_1_;
			fd_set wait_set;
			mutex process_mutex_;
			typedef boost::unique_lock<boost::mutex> thread_guard;
			boost::mutex thread_mutex_;
			std::vector<on_accept_function> callbacks_;
			std::vector<assign_function> assigners_;
			boost::asio::io_service *service_;
		}
;
	}
}


#endif
