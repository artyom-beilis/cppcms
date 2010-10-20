#ifndef BOOSTER_SMART_PTR_BAD_WEAK_PTR_HPP_INCLUDED
#define BOOSTER_SMART_PTR_BAD_WEAK_PTR_HPP_INCLUDED

//
//  boost/smart_ptr/bad_weak_ptr.hpp
//
//  Copyright (c) 2001, 2002, 2003 Peter Dimov and Multi Media Ltd.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <booster/backtrace.h>

#ifdef __BORLANDC__
# pragma warn -8026     // Functions with excep. spec. are not expanded inline
#endif

namespace booster
{

// The standard library that comes with Borland C++ 5.5.1, 5.6.4
// defines std::exception and its members as having C calling
// convention (-pc). When the definition of bad_weak_ptr
// is compiled with -ps, the compiler issues an error.
// Hence, the temporary #pragma option -pc below.

#if defined(__BORLANDC__) && __BORLANDC__ <= 0x564
# pragma option push -pc
#endif

///
/// An exeption that is throws in case of creating of shared_ptr from expired weak_ptr
///
class bad_weak_ptr: public booster::exception
{
public:

    virtual char const * what() const throw()
    {
        return "booster::bad_weak_ptr";
    }
};

#if defined(__BORLANDC__) && __BORLANDC__ <= 0x564
# pragma option pop
#endif

} // namespace boost

#ifdef __BORLANDC__
# pragma warn .8026     // Functions with excep. spec. are not expanded inline
#endif

#endif  // #ifndef BOOST_SMART_PTR_BAD_WEAK_PTR_HPP_INCLUDED
