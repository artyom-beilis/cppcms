#define CPPCMS_SOURCE
#include "thread_pool.h"
#include <list>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#if defined(CPPCMS_POSIX)
#include <signal.h>
#endif 

namespace cppcms {
namespace impl {
	class thread_pool : public util::noncopyable {
	public:

		bool cancel(int id) {
			boost::unique_lock<boost::mutex> lock(mutex_);
			queue_type::iterator p;
			for(p=queue_.begin();p!=queue_.end();++p) {
				if(p->first==id) {
					queue_.erase(p);
					return true;
				}
			}
			return false;
		}
		int post(util::callback0 const &job)
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			int id=job_id_++;
			queue_.push_back(std::make_pair(id,job));
			cond_.notify_one();
			return id;
		}
		thread_pool(int threads) :
			shut_down_(false),
			job_id_(0)
		{
			workers_.resize(threads);
			#if defined(CPPCMS_POSIX)
			sigset_t set,old;
			sigfillset(&set);
			pthread_sigmask(SIG_BLOCK,&set,&old);
			#endif
			for(int i=0;i<threads;i++) {
				workers_[i].reset(new boost::thread(boost::bind(&thread_pool::worker,this)));
			}
	
			#if defined(CPPCMS_POSIX)
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
						queue_.front().second.swap(job);
						queue_.pop_front();
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
		int job_id_;
		typedef std::list<std::pair<int,util::callback0> > queue_type;
		queue_type queue_;
		std::vector<boost::shared_ptr<boost::thread> > workers_;

	};
	


}

thread_pool::thread_pool(int n) :
	impl_(new impl::thread_pool(n))
{
}

int thread_pool::post(util::callback0  const &job)
{
	return impl_->post(job);
}

void thread_pool::stop()
{
	impl_->stop();
}

bool thread_pool::cancel(int id)
{
	return impl_->cancel(id);
}

thread_pool::~thread_pool()
{
}


} // cppcms


