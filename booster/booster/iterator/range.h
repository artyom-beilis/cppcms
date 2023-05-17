//
//  Copyright (C) 2020 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_ITERATOR_RANGE_H
#define BOOSTER_ITERATOR_RANGE_H

#include <iterator>

namespace booster { 
	using std::begin;
	using std::end;

	// rbegin, rend only in c++14 :-(
	template<typename C>
	auto rbegin(C &cnt) -> decltype(cnt.rbegin())
	{
		return cnt.rbegin();
	}
	template<typename C>
	auto rend(C &cnt) -> decltype(cnt.rend())
	{
		return cnt.rend();
	}
	template<typename C>
	auto rbegin(C const &cnt) -> decltype(cnt.rbegin())
	{
		return cnt.rbegin();
	}
	template<typename C>
	auto rend(C const &cnt) -> decltype(cnt.rend())
	{
		return cnt.rend();
	}
	template<typename T,size_t N>
	std::reverse_iterator<T*> rbegin(T (&a)[N])
	{
		return std::reverse_iterator<T*>(a+N);
	}
	template<typename T,size_t N>
	std::reverse_iterator<T*> rend(T (&a)[N])
	{
		return std::reverse_iterator<T*>(a);
	}

} // booster

#endif
