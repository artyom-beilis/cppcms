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
#include "atomic_counter.h"
#include "config.h"
#include <string.h>

#if defined(HAVE_WIN32_INTERLOCKED)

#   include <windows.h>
#   define cppcms_atomic_set(p,v) ((p)->l=v)
    long static cppcms_atomic_add_and_fetch_impl(volatile long *v,long d)
    {
	long old,prev;
	do{
		old = *v;
		prev = InterlockedCompareExchange(v,old+d,old);
	}
	while(prev != old);
	return old+d;
    }
#   define cppcms_atomic_add_and_fetch(p,d) cppcms_atomic_add_and_fetch_impl(&(p)->l,d)


#elif defined(HAVE_FREEBSD_ATOMIC)

#   include <sys/types.h>
#   include <machine/atomic.h>
#   define cppcms_atomic_set(p,v) ((p)->ui=v)
#   define cppcms_atomic_add_and_fetch(p,d) (atomic_fetchadd_int(&(p)->ui,d) + d)


#elif defined(HAVE_SOLARIS_ATOMIC)

#   include <atomic.h>
#   define cppcms_atomic_set(p,v) ((p)->ui=v)
#   define cppcms_atomic_add_and_fetch(p,d) (atomic_add_int_nv(&(p)->ui,d))


#elif defined(HAVE_MAC_OS_X_ATOMIC)

#   include <libkern/OSAtomic.h>
#   define cppcms_atomic_set(p,v) ((p)->i=v)
#   define cppcms_atomic_add_and_fetch(p,d) (OSAtomicAdd32(d,&(p)->i))

#elif defined HAVE_SYNC_FETCH_AND_ADD 

#   define cppcms_atomic_set(p,v) ((p)->l=v)
#   define cppcms_atomic_add_and_fetch(p,d) ( __sync_add_and_fetch(&(p)->i,d) )


#   elif defined(HAVE_GCC_BITS_EXCHANGE_AND_ADD)

#   include <bits/atomicity.h>
    using __gnu_cxx::__exchange_and_add;
#   define cppcms_atomic_set(p,v) ((p)->i=v)
#   define cppcms_atomic_add_and_fetch(p,d) ( __exchange_and_add(&(p)->i,d)+d )


#elif defined(HAVE_GCC_EXT_EXCHANGE_AND_ADD)

#   include <ext/atomicity.h>
    using __gnu_cxx::__exchange_and_add;
#   define cppcms_atomic_set(p,v) ((p)->i=v)
#   define cppcms_atomic_add_and_fetch(p,d) ( __exchange_and_add(&(p)->i,d)+d )


#else  // Failing back to pthreads
# include <pthread.h>
# define CPPCMS_PTHREAD_ATOMIC 
#endif


namespace cppcms {

#if !defined(CPPCMS_PTHREAD_ATOMIC)
	atomic_counter::atomic_counter(long value)  
	{
		memset(&value_,0,sizeof(value_));
		mutex_ = 0;
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



