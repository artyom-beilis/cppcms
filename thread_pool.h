#ifndef CPPCMS_THREAD_POOL_H
#define CPPCMS_THREAD_POOL_H

#include "defs.h"
#include "noncopyable.h"
#include "callback0.h"
#include "hold_ptr.h"

namespace cppcms {

	namespace impl {
		class thread_pool;
	}

	class CPPCMS_API thread_pool : public util::noncopyable {
	public:

		void post(util::callback0 const &job);	
		thread_pool(int threads);
		void stop();
		~thread_pool();

	private:

		util::hold_ptr<impl::thread_pool> impl_;
	};


} // cppcms



#endif

