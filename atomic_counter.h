#ifndef CPPCMS_ATOMIC_COUNT_H
#define CPPCMS_ATOMIC_COUNT_H

#include "defs.h"
#if !defined(CPPCMS_WIN32)
#	include <pthread.h>
#endif


namespace cppcms {

	class CPPCMS_API atomic_counter {
	public:
    		explicit atomic_counter( long v );
		~atomic_counter();
    		long operator++()
		{
			return inc();
		}
    		long operator--()
		{
			return dec();
		}
		operator long() const
		{
			return get();
		}
	private:
		long inc();
		long dec();
		long get() const;

		atomic_counter(atomic_counter const &);
		atomic_counter & operator=(atomic_counter const &);

		mutable long value_;
		#if !defined(CPPCMS_WIN32)
		// Is actually used for platforms without lock
		// it would not be used when atomic operations
		// availible
		mutable pthread_mutex_t mutex_;
		#endif
	};

} // cppcms

#endif


