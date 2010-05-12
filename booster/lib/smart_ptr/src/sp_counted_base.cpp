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

namespace booster
{

namespace detail
{

inline int atomic_exchange_and_add( int * pw, int dv )
{
    int r = *pw;
    *pw += dv;
    return r;
}

inline void atomic_increment( int * pw )
{
    ++*pw;
}

inline int atomic_conditional_increment( int * pw )
{
    int rv = *pw;
    if( rv != 0 ) ++*pw;
    return rv;
}

sp_counted_base::sp_counted_base(): use_count_( 1 ), weak_count_( 1 )
{
}

sp_counted_base::~sp_counted_base() // nothrow
{
}

void sp_counted_base::destroy() // nothrow
{
    delete this;
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
    atomic_increment( &weak_count_ );
}

void sp_counted_base::weak_release() // nothrow
{
    if( atomic_exchange_and_add( &weak_count_, -1 ) == 1 )
    {
        destroy();
    }
}

long sp_counted_base::use_count() const // nothrow
{
    // ATOMIC
    return use_count_;
}

} // namespace detail

} // namespace boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
