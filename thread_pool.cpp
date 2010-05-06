///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "thread_pool.h"
#include <list>
#include <vector>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/shared_ptr.hpp>
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/shared_ptr.hpp>
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

#include <booster/thread.h>

#if defined(CPPCMS_POSIX)
#include <signal.h>
#endif 

namespace cppcms {
namespace impl {
	class thread_pool : public util::noncopyable {
	public:

		bool cancel(int id) {
			booster::unique_lock<booster::mutex> lock(mutex_);
			queue_type::iterator p;
			for(p=queue_.begin();p!=queue_.end();++p) {
				if(p->first==id) {
					queue_.erase(p);
					return true;
				}
			}
			return false;
		}
		int post(function<void()> const &job)
		{
			booster::unique_lock<booster::mutex> lock(mutex_);
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
				workers_[i].reset(new booster::thread(boost::bind(&thread_pool::worker,this)));
			}
	
			#if defined(CPPCMS_POSIX)
			pthread_sigmask(SIG_SETMASK,&old,0);
			#endif
	
		}
		void stop()
		{
			{
				booster::unique_lock<booster::mutex> lock(mutex_);
				shut_down_=true;
				cond_.notify_all();
			}

			for(unsigned i=0;i<workers_.size();i++) {
				boost::shared_ptr<booster::thread> thread=workers_[i];
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
				function<void()> job;

				{
					booster::unique_lock<booster::mutex> lock(mutex_);
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

				if(!job.empty())
					job();
			} 	
		}

	
		booster::mutex mutex_;
		booster::condition_variable cond_;

		bool shut_down_;	
		int job_id_;
		typedef std::list<std::pair<int,function<void()> > > queue_type;
		queue_type queue_;
		std::vector<boost::shared_ptr<booster::thread> > workers_;

	};
	


}

thread_pool::thread_pool(int n) :
	impl_(new impl::thread_pool(n))
{
}

int thread_pool::post(function<void()>  const &job)
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


