//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/atomic_counter.h>
#include <booster/build_config.h>
#include <string.h>

#if defined(BOOSTER_WIN32)

#   include <windows.h>
#   define booster_atomic_set(p,v) ((p)->l=v)
    long static booster_atomic_add_and_fetch_impl(volatile long *v,long d)
    {
	long old,prev;
	do{
		old = *v;
		prev = InterlockedCompareExchange(v,old+d,old);
	}
	while(prev != old);
	return old+d;
    }
#   define booster_atomic_add_and_fetch(p,d) booster_atomic_add_and_fetch_impl(&(p)->l,d)


#elif defined(BOOSTER_HAVE_FREEBSD_ATOMIC)

#   include <sys/types.h>
#   include <machine/atomic.h>
#   define booster_atomic_set(p,v) ((p)->ui=v)
#   define booster_atomic_add_and_fetch(p,d) (atomic_fetchadd_int(&(p)->ui,d) + d)


#elif defined(BOOSTER_HAVE_SOLARIS_ATOMIC)

#   include <atomic.h>
#   define booster_atomic_set(p,v) ((p)->ui=v)
#   define booster_atomic_add_and_fetch(p,d) (atomic_add_int_nv(&(p)->ui,d))


#elif defined(BOOSTER_HAVE_MAC_OS_X_ATOMIC)

#   include <libkern/OSAtomic.h>
#   define booster_atomic_set(p,v) ((p)->i=v)
#   define booster_atomic_add_and_fetch(p,d) (OSAtomicAdd32(d,const_cast<int32_t *>(&(p)->i)))

#elif defined BOOSTER_HAS_GCC_SYNC

#   define booster_atomic_set(p,v) ((p)->l=v)
#   define booster_atomic_add_and_fetch(p,d) ( __sync_add_and_fetch(&(p)->i,d) )


#   elif defined(BOOSTER_HAVE_GCC_BITS_EXCHANGE_AND_ADD)

#   include <bits/atomicity.h>
    using __gnu_cxx::__exchange_and_add;
#   define booster_atomic_set(p,v) ((p)->i=v)
#   define booster_atomic_add_and_fetch(p,d) ( __exchange_and_add(&(p)->i,d)+d )


#elif defined(BOOSTER_HAVE_GCC_EXT_EXCHANGE_AND_ADD)

#   include <ext/atomicity.h>
    using __gnu_cxx::__exchange_and_add;
#   define booster_atomic_set(p,v) ((p)->i=v)
#   define booster_atomic_add_and_fetch(p,d) ( __exchange_and_add(&(p)->i,d)+d )


#else  // Failing back to pthreads
# include <pthread.h>
# define BOOSTER_PTHREAD_ATOMIC 
#endif


namespace booster {

#if !defined(BOOSTER_PTHREAD_ATOMIC)
	atomic_counter::atomic_counter(long value)  
	{
		memset(&value_,0,sizeof(value_));
		mutex_ = 0;
		booster_atomic_set(&value_,value);
	}

	atomic_counter::~atomic_counter()
	{
	}
	
	long atomic_counter::inc()
	{
		return booster_atomic_add_and_fetch( &value_, 1 );
	}

	long atomic_counter::dec()
	{
		return booster_atomic_add_and_fetch( &value_, -1 );
	}

	long atomic_counter::get() const
	{
		return booster_atomic_add_and_fetch( &value_, 0 );
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


} // booster



