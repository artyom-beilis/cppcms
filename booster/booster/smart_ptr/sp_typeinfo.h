#ifndef BOOSTER_DETAIL_SP_TYPEINFO_HPP_INCLUDED
#define BOOSTER_DETAIL_SP_TYPEINFO_HPP_INCLUDED

//  detail/sp_typeinfo.hpp
//
//  Copyright 2007 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <booster/config.h>


#include <typeinfo>

namespace booster
{

namespace detail
{

typedef std::type_info sp_typeinfo;

} // namespace detail

} // namespace boost

#define BOOSTER_SP_TYPEID(T) typeid(T)


#endif  // #ifndef BOOST_DETAIL_SP_TYPEINFO_HPP_INCLUDED
