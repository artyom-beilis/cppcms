#define CPPCMS_SOURCE
#include "atomic_counter.h"
#include "config.h"

#ifdef CPPCMS_WIN32
#  include <windows.h>
#else
#  if __GNUC__ * 100 + __GNUC_MINOR__ >= 402
#    include <ext/atomicity.h>
#  else
#    include <bits/atomicity.h>
#  endif
#endif



namespace cppcms {

#if defined(CPPCMS_WIN32) || defined(HAVE_GCC_EXCHANGE_AND_ADD) || defined(HAVE_SYNC_FETCH_AND_ADD)
	atomic_counter::atomic_counter(long value) : value_(value)
	{
	}

	atomic_counter::~atomic_counter()
	{
	}

#else
	atomic_counter::atomic_counter(long value) :
		value_(value)
	{
		pthread_mutex_init(&mutex_,0);
	}

	atomic_counter::~atomic_counter()
	{
		pthread_mutex_destroy(&mutex_);
	}
#endif


#ifdef CPPCMS_WIN32
	long atomic_counter::inc()
	{
		return InterlockedIncrement(&value_);
	}
	long atomic_counter::dec()
	{
		return InterlockedDecrement(&value_);
	}

	long atomic_counter::get() const
	{
		return *reinterpret_cast<volatile long *>(&value_);
	}

#elif defined(HAVE_SYNC_FETCH_AND_ADD)

	long atomic_counter::inc()
	{
		return __sync_add_and_fetch( &value_, 1 );
	}

	long atomic_counter::dec()
	{
		return __sync_add_and_fetch( &value_, -1 );
	}

	long atomic_counter::get()
	{
		return __sync_fetch_and_add( &value_, 0 );
	}

#elif defined(HAVE_GCC_EXCHANGE_AND_ADD)
	#if defined(__GLIBCXX__) // g++ 3.4+

	using __gnu_cxx::__atomic_add;
	using __gnu_cxx::__exchange_and_add;

	#endif

	long atomic_counter::inc()
	{
		return __exchange_and_add( &value_, +1 ) + 1;
	}

	long atomic_counter::dec()
	{
		return __exchange_and_add( &value_, -1 ) - 1;
	}

	long atomic_counter::get() const
	{
		return __exchange_and_add( &value_, 0 );
	}




#else // pthreads

	long atomic_counter::inc()
	{
		pthread_mutex_lock(&mutex_);
		long result= ++value_;
		pthread_mutex_unlock(&mutex_);
		return result;
	}

	long atomic_counter::dec()
	{
		pthread_mutex_lock(&mutex_);
		long result= --value_;
		pthread_mutex_unlock(&mutex_);
		return result;
	}

	long atomic_counter::get() const
	{
		pthread_mutex_lock(&mutex_);
		long result= value_;
		pthread_mutex_unlock(&mutex_);
		return result;
	}


#endif


} // cppcms



