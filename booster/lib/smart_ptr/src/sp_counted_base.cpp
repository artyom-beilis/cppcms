#define BOOSTER_SOURCE
//
//  detail/sp_counted_base_spin.hpp - spinlock pool atomic emulation
//
//  Copyright (c) 2001, 2002, 2003 Peter Dimov and Multi Media Ltd.
//  Copyright 2004-2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <booster/smart_ptr/sp_counted_base.h>
#include <booster/build_config.h>

#undef SP_ATOMIC


#if defined BOOSTER_WIN32

///////////////
//  Windows
///////////////

#   include <windows.h>
#   define SP_ATOMIC

namespace booster { 
    namespace detail {

        static bool atomic_cas(sp_counted_base_atomic_type volatile *pw,int old_value,int new_value)
        {
            return InterlockedCompareExchange(&pw->li,new_value,old_value)==old_value;
        }

        static int atomic_get(sp_counted_base_atomic_type volatile *pw)
        {
            long v=pw->li;
            while(!atomic_cas(pw,v,v)) {
                v=pw->li;
            }
            return v;
        }

        static void atomic_set(sp_counted_base_atomic_type volatile *pw,int v)
        {
            long vo=pw->li;
            while(!atomic_cas(pw,vo,v)) {
                vo=pw->li;
            }
        }
        static int atomic_exchange_and_add(sp_counted_base_atomic_type volatile * pw, int dv)
        {
            long vo=pw->li,tmp;
            for(;;){
                if((tmp=InterlockedCompareExchange(&pw->li,vo+dv,vo)) == vo)
					break;
				vo = tmp;
            }
            return tmp;
        }

        static void atomic_increment(sp_counted_base_atomic_type volatile *pw)
        {
            InterlockedIncrement(&pw->li);
        }

    } //details
}  // booster

#elif defined BOOSTER_HAVE_MAC_OS_X_ATOMIC

////////////
// Mac OS X
////////////

namespace booster { 
    namespace detail {

#   include <libkern/OSAtomic.h>
#   define SP_ATOMIC

        static bool atomic_cas(sp_counted_base_atomic_type volatile *pw,int old_value,int new_value)
        {
            // Older versions get non-volatile parameter
            return OSAtomicCompareAndSwap32(old_value,new_value,const_cast<int32_t *>(&pw->i));
        }
        static int atomic_get(sp_counted_base_atomic_type volatile *pw)
        {
            OSMemoryBarrier();
            return pw->i;
        }

        static void atomic_set(sp_counted_base_atomic_type volatile *pw,int v)
        {
            pw->i=v;
            OSMemoryBarrier();
        }
        static int atomic_exchange_and_add(sp_counted_base_atomic_type volatile * pw, int dv)
        {
            // Older versions get non-volatile parameter
            return OSAtomicAdd32(dv,const_cast<int32_t *>(&pw->i))-dv;
        }
        static void atomic_increment(sp_counted_base_atomic_type volatile *pw)
        {
            atomic_exchange_and_add(pw,1);
        }
    } // detail
} // booster


#elif defined BOOSTER_HAVE_FREEBSD_ATOMIC

////////////
// FreeBSD
///////////

namespace booster { 
    namespace detail {

#   include <sys/types.h>
#   include <machine/atomic.h>
#   define SP_ATOMIC

        static bool atomic_cas(sp_counted_base_atomic_type volatile *pw,int old_value,int new_value)
        {
            return atomic_cmpset_int(&pw->ui,old_value,new_value);
        }
        static int atomic_get(sp_counted_base_atomic_type volatile *pw)
        {
            return atomic_load_acq_int(&pw->ui);
        }

        static void atomic_set(sp_counted_base_atomic_type volatile *pw,int v)
        {
            atomic_store_rel_int(&pw->ui,v);
        }
        static int atomic_exchange_and_add(sp_counted_base_atomic_type volatile * pw, int dv)
        {
            return atomic_fetchadd_int(&pw->ui,dv);
        }
        static void atomic_increment(sp_counted_base_atomic_type volatile *pw)
        {
            atomic_exchange_and_add(pw,1);
        }
    } // detail
} // booster
#elif defined BOOSTER_HAVE_SOLARIS_ATOMIC
///////////////
// Sun Solaris
///////////////

#   include <atomic.h>
#   define SP_ATOMIC

namespace booster { 
    namespace detail {


        static bool atomic_cas(sp_counted_base_atomic_type volatile *pw,int old_value,int new_value)
        {
            return atomic_cas_uint(&pw->ui,unsigned(old_value),unsigned(new_value))==unsigned(old_value);
        }
        static int atomic_get(sp_counted_base_atomic_type volatile *pw)
        {
            membar_consumer();
            return pw->i;
        }

        static void atomic_set(sp_counted_base_atomic_type volatile *pw,int v)
        {
            membar_producer();
            pw->i=v;
        }
        static int atomic_exchange_and_add(sp_counted_base_atomic_type volatile * pw, int dv)
        {
            return atomic_add_int_nv(&pw->ui,dv)-dv;
        }
        static void atomic_increment(sp_counted_base_atomic_type volatile *pw)
        {
            atomic_add_int(&pw->ui,1);
        }
    } // detail
} // booster

#elif defined BOOSTER_HAS_GCC_SYNC
//////////////////////////
// GCC __sync_
/////////////////////////

#   define SP_ATOMIC
namespace booster { 
    namespace detail {

        static int atomic_get(sp_counted_base_atomic_type volatile *pw)
        {
            __sync_synchronize();
            return pw->i;
        }
        static void atomic_set(sp_counted_base_atomic_type volatile *pw,int v)
        {
            pw->i=v;
            __sync_synchronize();
        }
        static bool atomic_cas(sp_counted_base_atomic_type volatile *pw,int old_value,int new_value)
        {
            return __sync_bool_compare_and_swap(&pw->i,old_value,new_value);
        }
        static int atomic_exchange_and_add(sp_counted_base_atomic_type volatile * pw, int dv)
        {
            return __sync_fetch_and_add(&pw->i,dv);
        }
        static void atomic_increment( sp_counted_base_atomic_type volatile * pw)
        {
            atomic_exchange_and_add(pw,1);
        }
    }
}

#endif // Any atomic op


//////////////
// Implementation
////////////////

namespace booster
{

    namespace detail
    {

#if defined SP_ATOMIC

        // We have full atomic operations support

        inline int atomic_conditional_increment( sp_counted_base_atomic_type * pw)
        {
            for(;;) {
                int rv = atomic_get(pw);
                if(rv == 0)
                    return 0;
                if(atomic_cas(pw,rv,rv+1))
                    return rv;
            }
        }

        sp_counted_base::sp_counted_base()
        {
            atomic_set(&use_count_,1);
            atomic_set(&weak_count_,1);
        }

        sp_counted_base::~sp_counted_base() // nothrow
        {
        }

        void sp_counted_base::add_ref_copy()
        {
            atomic_increment( &use_count_ );
        }

        bool sp_counted_base::add_ref_lock() // true on success
        {
            return atomic_conditional_increment( &use_count_ ) != 0;
        }

        void sp_counted_base::release() // nothrow
        {
            if( atomic_exchange_and_add( &use_count_, -1 ) == 1 )
            {
                dispose();
                weak_release();
            }
        }

        void sp_counted_base::weak_add_ref() // nothrow
        {
            atomic_increment( &weak_count_);
        }

        void sp_counted_base::weak_release() // nothrow
        {
            if( atomic_exchange_and_add( &weak_count_, -1) == 1 )
            {
                destroy();
            }
        }

        long sp_counted_base::use_count() const // nothrow
        {
            return atomic_get(&use_count_);
        }

#else // PTHREAD

        inline int atomic_exchange_and_add( sp_counted_base_atomic_type * pw, int dv, pthread_mutex_t *m)
        {
            pthread_mutex_lock(m);
            int r = pw->i;
            pw->i += dv;
            pthread_mutex_unlock(m);
            return r;
        }

        inline void atomic_increment( sp_counted_base_atomic_type * pw , pthread_mutex_t *m)
        {
            pthread_mutex_lock(m);
            ++(pw->i);
            pthread_mutex_unlock(m);
        }

        inline int atomic_conditional_increment( sp_counted_base_atomic_type * pw , pthread_mutex_t *m)
        {
            pthread_mutex_lock(m);
            int rv = pw->i;
            if( rv != 0 ) ++(pw->i);
            pthread_mutex_unlock(m);
            return rv;
        }

        inline void atomic_set(sp_counted_base_atomic_type *pw,int v, pthread_mutex_t *m)
        {
            pthread_mutex_lock(m);
            pw->i=v;
            pthread_mutex_unlock(m);
        }

        inline int atomic_get(sp_counted_base_atomic_type *pw, pthread_mutex_t *m)
        {
            pthread_mutex_lock(m);
            int r=pw->i;
            pthread_mutex_unlock(m);
            return r;
        }


        sp_counted_base::sp_counted_base()
        {
            pthread_mutex_init(&lock_,0);
            atomic_set(&use_count_,1,&lock_);
            atomic_set(&weak_count_,1,&lock_);
        }

        sp_counted_base::~sp_counted_base() // nothrow
        {
            pthread_mutex_destroy(&lock_);
        }

        void sp_counted_base::add_ref_copy()
        {
            atomic_increment( &use_count_ ,&lock_);
        }

        bool sp_counted_base::add_ref_lock() // true on success
        {
            return atomic_conditional_increment( &use_count_ ,&lock_) != 0;
        }

        void sp_counted_base::release() // nothrow
        {
            if( atomic_exchange_and_add( &use_count_, -1 ,&lock_) == 1 )
            {
                dispose();
                weak_release();
            }
        }

        void sp_counted_base::weak_add_ref() // nothrow
        {
            atomic_increment( &weak_count_,&lock_ );
        }

        void sp_counted_base::weak_release() // nothrow
        {
            if( atomic_exchange_and_add( &weak_count_, -1 ,&lock_) == 1 )
            {
                destroy();
            }
        }

        long sp_counted_base::use_count() const // nothrow
        {
            return atomic_get(&use_count_,&lock_);
        }

#endif // pthreads

        void sp_counted_base::destroy() // nothrow
        {
            delete this;
        }


    } // namespace detail

} // namespace boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
