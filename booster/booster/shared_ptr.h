#ifndef BOOSTER_SMART_PTR_SHARED_PTR_HPP_INCLUDED
#define BOOSTER_SMART_PTR_SHARED_PTR_HPP_INCLUDED

//
//  shared_ptr.hpp
//
//  (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
//  Copyright (c) 2001-2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/shared_ptr.htm for documentation.
//

#include <booster/config.h>   // for broken compiler workarounds

#include <memory>
#include <cassert>
#include <booster/checked_delete.h>
#include <booster/smart_ptr/shared_count.h>
#include <booster/smart_ptr/sp_convertible.h>

#include <algorithm>            // for std::swap
#include <functional>           // for std::less
#include <typeinfo>             // for std::bad_cast

#include <iosfwd>               // for std::basic_ostream

#ifdef BOOSTER_MSVC  // moved here to work around VC++ compiler crash
# pragma warning(push)
# pragma warning(disable:4284) // odd return type for operator->
#endif

namespace booster
{

template<class T> class shared_ptr;
template<class T> class weak_ptr;
template<class T> class enable_shared_from_this;
template<class T> class enable_shared_from_this2;

namespace detail
{

struct static_cast_tag {};
struct const_cast_tag {};
struct dynamic_cast_tag {};
struct polymorphic_cast_tag {};

template<class T> struct shared_ptr_traits
{
    typedef T & reference;
};

template<> struct shared_ptr_traits<void>
{
    typedef void reference;
};

template<> struct shared_ptr_traits<void const>
{
    typedef void reference;
};

template<> struct shared_ptr_traits<void volatile>
{
    typedef void reference;
};

template<> struct shared_ptr_traits<void const volatile>
{
    typedef void reference;
};

// enable_shared_from_this support

template< class X, class Y, class T > inline void sp_enable_shared_from_this( booster::shared_ptr<X> const * ppx, Y const * py, booster::enable_shared_from_this< T > const * pe )
{
    if( pe != 0 )
    {
        pe->_internal_accept_owner( ppx, const_cast< Y* >( py ) );
    }
}

template< class X, class Y, class T > inline void sp_enable_shared_from_this( booster::shared_ptr<X> * ppx, Y const * py, booster::enable_shared_from_this2< T > const * pe )
{
    if( pe != 0 )
    {
        pe->_internal_accept_owner( ppx, const_cast< Y* >( py ) );
    }
}

inline void sp_enable_shared_from_this( ... )
{
}


// rvalue auto_ptr support based on a technique by Dave Abrahams

template< class T, class R > struct sp_enable_if_auto_ptr
{
};

template< class T, class R > struct sp_enable_if_auto_ptr< std::auto_ptr< T >, R >
{
    typedef R type;
}; 

} // namespace detail


//
//  shared_ptr
//
//  An enhanced relative of scoped_ptr with reference counted copy semantics.
//  The object pointed to is deleted when the last shared_ptr pointing to it
//  is destroyed or reset.
//

template<class T> class shared_ptr
{
private:

    // Borland 5.5.1 specific workaround
    typedef shared_ptr<T> this_type;

public:

    typedef T element_type;
    typedef T value_type;
    typedef T * pointer;
    typedef typename booster::detail::shared_ptr_traits<T>::reference reference;

    shared_ptr(): px(0), pn() // never throws in 1.30+
    {
    }

    template<class Y>
    explicit shared_ptr( Y * p ): px( p ), pn( p ) // Y must be complete
    {
        booster::detail::sp_enable_shared_from_this( this, p, p );
    }

    //
    // Requirements: D's copy constructor must not throw
    //
    // shared_ptr will release p by calling d(p)
    //

    template<class Y, class D> shared_ptr(Y * p, D d): px(p), pn(p, d)
    {
        booster::detail::sp_enable_shared_from_this( this, p, p );
    }

    // As above, but with allocator. A's copy constructor shall not throw.

    template<class Y, class D, class A> shared_ptr( Y * p, D d, A a ): px( p ), pn( p, d, a )
    {
        booster::detail::sp_enable_shared_from_this( this, p, p );
    }

//  generated copy constructor, destructor are fine

    template<class Y>
    explicit shared_ptr(weak_ptr<Y> const & r): pn(r.pn) // may throw
    {
        // it is now safe to copy r.px, as pn(r.pn) did not throw
        px = r.px;
    }

    template<class Y>
    shared_ptr( weak_ptr<Y> const & r, booster::detail::sp_nothrow_tag ): px( 0 ), pn( r.pn, booster::detail::sp_nothrow_tag() ) // never throws
    {
        if( !pn.empty() )
        {
            px = r.px;
        }
    }

    template<class Y>
    shared_ptr( shared_ptr<Y> const & r, typename detail::sp_enable_if_convertible<Y,T>::type = detail::sp_empty() )
    : px( r.px ), pn( r.pn ) // never throws
    {
    }

    // aliasing
    template< class Y >
    shared_ptr( shared_ptr<Y> const & r, T * p ): px( p ), pn( r.pn ) // never throws
    {
    }

    template<class Y>
    shared_ptr(shared_ptr<Y> const & r, booster::detail::static_cast_tag): px(static_cast<element_type *>(r.px)), pn(r.pn)
    {
    }

    template<class Y>
    shared_ptr(shared_ptr<Y> const & r, booster::detail::const_cast_tag): px(const_cast<element_type *>(r.px)), pn(r.pn)
    {
    }

    template<class Y>
    shared_ptr(shared_ptr<Y> const & r, booster::detail::dynamic_cast_tag): px(dynamic_cast<element_type *>(r.px)), pn(r.pn)
    {
        if(px == 0) // need to allocate new counter -- the cast failed
        {
            pn = booster::detail::shared_count();
        }
    }

    template<class Y>
    shared_ptr(shared_ptr<Y> const & r, booster::detail::polymorphic_cast_tag): px(dynamic_cast<element_type *>(r.px)), pn(r.pn)
    {
        if(px == 0)
        {
            throw std::bad_cast();
        }
    }

    template<class Y>
    explicit shared_ptr(std::auto_ptr<Y> & r): px(r.get()), pn()
    {
        Y * tmp = r.get();
        pn = booster::detail::shared_count(r);
        booster::detail::sp_enable_shared_from_this( this, tmp, tmp );
    }

    template<class Ap>
    explicit shared_ptr( Ap r, typename booster::detail::sp_enable_if_auto_ptr<Ap, int>::type = 0 ): px( r.get() ), pn()
    {
        typename Ap::element_type * tmp = r.get();
        pn = booster::detail::shared_count( r );
        booster::detail::sp_enable_shared_from_this( this, tmp, tmp );
    }


    // assignment

    shared_ptr & operator=( shared_ptr const & r ) // never throws
    {
        this_type(r).swap(*this);
        return *this;
    }

    template<class Y>
    shared_ptr & operator=(shared_ptr<Y> const & r) // never throws
    {
        this_type(r).swap(*this);
        return *this;
    }


    template<class Y>
    shared_ptr & operator=( std::auto_ptr<Y> & r )
    {
        this_type(r).swap(*this);
        return *this;
    }

    template<class Ap>
    typename booster::detail::sp_enable_if_auto_ptr< Ap, shared_ptr & >::type operator=( Ap r )
    {
        this_type( r ).swap( *this );
        return *this;
    }

// Move support

    void reset() // never throws in 1.30+
    {
        this_type().swap(*this);
    }

    template<class Y> void reset(Y * p) // Y must be complete
    {
        assert(p == 0 || p != px); // catch self-reset errors
        this_type(p).swap(*this);
    }

    template<class Y, class D> void reset( Y * p, D d )
    {
        this_type( p, d ).swap( *this );
    }

    template<class Y, class D, class A> void reset( Y * p, D d, A a )
    {
        this_type( p, d, a ).swap( *this );
    }

    template<class Y> void reset( shared_ptr<Y> const & r, T * p )
    {
        this_type( r, p ).swap( *this );
    }

    reference operator* () const // never throws
    {
        assert(px != 0);
        return *px;
    }

    T * operator-> () const // never throws
    {
        assert(px != 0);
        return px;
    }

    T * get() const // never throws
    {
        return px;
    }

// implicit conversion to "bool"
#include <booster/smart_ptr/operator_bool.h>

    bool unique() const // never throws
    {
        return pn.unique();
    }

    long use_count() const // never throws
    {
        return pn.use_count();
    }

    void swap(shared_ptr<T> & other) // never throws
    {
        std::swap(px, other.px);
        pn.swap(other.pn);
    }

    template<class Y> bool _internal_less(shared_ptr<Y> const & rhs) const
    {
        return pn < rhs.pn;
    }

    void * _internal_get_deleter( detail::sp_typeinfo const & ti ) const
    {
        return pn.get_deleter( ti );
    }

    bool _internal_equiv( shared_ptr const & r ) const
    {
        return px == r.px && pn == r.pn;
    }

// Tasteless as this may seem, making all members public allows member templates
// to work in the absence of member template friends. (Matthew Langston)

private:

    template<class Y> friend class shared_ptr;
    template<class Y> friend class weak_ptr;

    T * px;                     // contained pointer
    booster::detail::shared_count pn;    // reference counter

};  // shared_ptr

template<class T, class U> inline bool operator==(shared_ptr<T> const & a, shared_ptr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(shared_ptr<T> const & a, shared_ptr<U> const & b)
{
    return a.get() != b.get();
}

template<class T, class U> inline bool operator<(shared_ptr<T> const & a, shared_ptr<U> const & b)
{
    return a._internal_less(b);
}

template<class T> inline void swap(shared_ptr<T> & a, shared_ptr<T> & b)
{
    a.swap(b);
}

template<class T, class U> shared_ptr<T> static_pointer_cast(shared_ptr<U> const & r)
{
    return shared_ptr<T>(r, booster::detail::static_cast_tag());
}

template<class T, class U> shared_ptr<T> const_pointer_cast(shared_ptr<U> const & r)
{
    return shared_ptr<T>(r, booster::detail::const_cast_tag());
}

template<class T, class U> shared_ptr<T> dynamic_pointer_cast(shared_ptr<U> const & r)
{
    return shared_ptr<T>(r, booster::detail::dynamic_cast_tag());
}

// shared_*_cast names are deprecated. Use *_pointer_cast instead.

template<class T, class U> shared_ptr<T> shared_static_cast(shared_ptr<U> const & r)
{
    return shared_ptr<T>(r, booster::detail::static_cast_tag());
}

template<class T, class U> shared_ptr<T> shared_dynamic_cast(shared_ptr<U> const & r)
{
    return shared_ptr<T>(r, booster::detail::dynamic_cast_tag());
}

template<class T, class U> shared_ptr<T> shared_polymorphic_cast(shared_ptr<U> const & r)
{
    return shared_ptr<T>(r, booster::detail::polymorphic_cast_tag());
}

template<class T, class U> shared_ptr<T> shared_polymorphic_downcast(shared_ptr<U> const & r)
{
    assert(dynamic_cast<T *>(r.get()) == r.get());
    return shared_static_cast<T>(r);
}

// get_pointer() enables boost::mem_fn to recognize shared_ptr

template<class T> inline T * get_pointer(shared_ptr<T> const & p)
{
    return p.get();
}

// operator<<

template<class Y> std::ostream & operator<< (std::ostream & os, shared_ptr<Y> const & p)
{
    os << p.get();
    return os;
}

// get_deleter

template<class D, class T> D * get_deleter(shared_ptr<T> const & p)
{
    return static_cast<D *>(p._internal_get_deleter(BOOSTER_SP_TYPEID(D)));
}

} // namespace boost

#ifdef BOOSTER_MSVC
# pragma warning(pop)
#endif

#endif  // #ifndef BOOST_SMART_PTR_SHARED_PTR_HPP_INCLUDED
