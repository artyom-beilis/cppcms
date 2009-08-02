#define CPPCMS_SOURCE
#include "thread_pool.h"
#include <queue>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

namespace cppcms {
namespace impl {
	class thread_pool : public util::noncopyable {
	public:

		void post(util::callback0 const &job)
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			queue_.push(job);
			cond_.notify_one();
		}
		thread_pool(int threads)
		{
			workers_.resize(threads);
			#ifndef _WIN32
			sigset_t set,old;
			sigfillset(&set);
			pthread_sigmask(SIG_BLOCK,&set,&old);
			#endif
			for(int i=0;i<threads;i++) {
				workers_[i].reset(new boost::thread(boost::bind(&thread_pool::worker,this)));
			}
	
			#ifndef _WIN32
			pthread_sigmask(SIG_SETMASK,&old,0);
			#endif
	
		}
		void stop()
		{
			{
				boost::unique_lock<boost::mutex> lock(mutex_);
				shut_down_=true;
				cond_.notify_all();
			}

			for(unsigned i=0;i<workers_.size();i++) {
				boost::shared_ptr<boost::thread> thread=workers_[i];
				workers_[i].reset();
				if(thread)
					thread->join();
			}
		}
		~thread_pool()
		{
			try {
				stop();
			}
			catch(...)
			{
			}
		}

	private:

		void worker()
		{
			for(;;) {
				util::callback0 job;

				{
					boost::unique_lock<boost::mutex> lock(mutex_);
					if(shut_down_)
						return;
					if(!queue_.empty()) {
						queue_.front().swap(job);
						queue_.pop();
					}
					else {
						cond_.wait(lock);
					}
				}

				job();
			} 	
		}

	
		boost::mutex mutex_;
		boost::condition_variable cond_;

		bool shut_down_;	
		std::queue<util::callback0> queue_;
		std::vector<boost::shared_ptr<boost::thread> > workers_;

	};
	


}

thread_pool::thread_pool(int n) :
	impl_(new impl::thread_pool(n))
{
}

void thread_pool::post(util::callback0  const &job)
{
	impl_->post(job);
}

void thread_pool::stop()
{
	impl_->stop();
}

thread_pool::~thread_pool()
{
}


} // cppcms


