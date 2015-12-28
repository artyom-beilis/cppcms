//
// Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOSTER_TYPE_TRAITS_H
#define BOOSTER_TYPE_TRAITS_H

#include <booster/traits/enable_if.h>
#include <booster/traits/is_base_of.h>


namespace booster{
	template<typename T>
	struct remove_const
	{
		typedef T type;
	};

	template<typename T>
	struct remove_const<T const>
	{
		typedef T type;
	};


	template<typename T>
	struct remove_reference
	{
		typedef T type;
	};

	template<typename T>
	struct remove_reference<T &>
	{
		typedef T type;
	};

	template<typename T>
	struct remove_const_reference {
		typedef typename remove_const<typename remove_reference<T>::type>::type type;
	};



} // booster

#endif


