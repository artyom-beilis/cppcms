#define CPPCMS_SOURCE
#include "atomic_counter.h"
#include "config.h"

#if defined HAVE_SYNC_FETCH_AND_ADD 
# define cppcms_atomic_set(p,v) ((p)->l=v)
# define cppcms_atomic_add_and_fetch(p,d) ( __sync_add_and_fetch(&(p)->i,d) )
#elif defined(CPPCMS_WIN32)
# include <windows.h>
# define cppcms_atomic_set(p,v) ((p)->l=v)
# define cppcms_atomic_add_and_fetch(p,d) InterlockedIncrement(&(p)->l,d)
#elif defined(HAVE_FREEBSD_ATOMIC)
# include <sys/types.h>
# include <machine/atomic.h>
# define cppcms_atomic_set(p,v) ((p)->ui=v)
# define cppcms_atomic_add_and_fetch(p,d) (atomic_fetchadd_int(&(p)->ui,d) + d)
#elif defined(HAVE_SOLARIS_ATOMIC)
# include <atomic.h>
# define cppcms_atomic_set(p,v) ((p)->ui=v)
# define cppcms_atomic_add_and_fetch(p,d) (atomic_add_int_nv(&(p)->ui,d))
#elif defined(HAVE_MAC_OS_X_ATOMIC)
# include <libkern/OSAtomic.h>
# define cppcms_atomic_set(p,v) ((p)->i=v)
# define cppcms_atomic_add_and_fetch(p,d) (OSAtomicAdd32(d,&(p)->i))
#else
# include <pthread.h>
# define CPPCMS_PTHREAD_ATOMIC 
#endif


namespace cppcms {

#if !defined(CPPCMS_PTHREAD_ATOMIC)
	atomic_counter::atomic_counter(long value) 
	{
		cppcms_atomic_set(&value_,value);
	}

	atomic_counter::~atomic_counter()
	{
	}
	
	long atomic_counter::inc()
	{
		return cppcms_atomic_add_and_fetch( &value_, 1 );
	}

	long atomic_counter::dec()
	{
		return cppcms_atomic_add_and_fetch( &value_, -1 );
	}

	long atomic_counter::get() const
	{
		return cppcms_atomic_add_and_fetch( &value_, 0 );
	}

#else

	#define MUTEX (reinterpret_cast<pthread_mutex_t *>(mutex_))
	atomic_counter::atomic_counter(long value) 
	{
		mutex_ = new pthread_mutex_t; 
		pthread_mutex_init(MUTEX,0);
		value_.l=value;
	}

	atomic_counter::~atomic_counter()
	{
		pthread_mutex_destroy(MUTEX);
		delete MUTEX;
		mutex_ = 0;
	}

	long atomic_counter::inc()
	{
		pthread_mutex_lock(MUTEX);
		long result= ++value_.l;
		pthread_mutex_unlock(MUTEX);
		return result;
	}

	long atomic_counter::dec()
	{
		pthread_mutex_lock(MUTEX);
		long result= --value_.l;
		pthread_mutex_unlock(MUTEX);
		return result;
	}

	long atomic_counter::get() const
	{
		pthread_mutex_lock(MUTEX);
		long result= value_.l;
		pthread_mutex_unlock(MUTEX);
		return result;
	}


#endif


} // cppcms



