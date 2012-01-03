///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_THREAD_POOL_H
#define CPPCMS_THREAD_POOL_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/function.h>
#include <booster/hold_ptr.h>

namespace cppcms {

	namespace impl {
		class thread_pool;
	}

	///
	/// \brief This class provides an access to the thread pool where all CppCMS synchronous
	/// applications are executed.
	///
	/// Users of asynchronous applications or tasks my send their jobs for the execution in the
	/// thread_pool
	/// 
	class CPPCMS_API thread_pool : public booster::noncopyable {
	public:

		///
		/// Post a request for execution of \a job in the pool. Received integer is special job identification
		/// number that can be used to remove the job from the queue.
		///
		int post(booster::function<void()> const &job);	

		///
		/// Cancel the job using id received from post() function
		///
		/// Returns true if the job was removed from the queue and false if the operation failed: the job execution is in the process
		/// or completed
		///
		/// Note: the id is just a rolling number and job ids may be repeated once in a while, so it is good idea to check if the job
		/// was completed before cancelling the job (even it is unlikely the 4 billion jobs would be executed in small period of time.
		///
		bool cancel(int id);

		/// \cond INTERNAL
		thread_pool(int threads);
		void stop();
		~thread_pool();
		//// \endcond

	private:

		booster::hold_ptr<impl::thread_pool> impl_;
	};


} // cppcms



#endif

