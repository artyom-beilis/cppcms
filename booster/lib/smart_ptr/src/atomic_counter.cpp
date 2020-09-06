//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/atomic_counter.h>

namespace booster {

	atomic_counter::atomic_counter(long value) : value_(value)
	{
	}

	atomic_counter::~atomic_counter()
	{
	}
	
	long atomic_counter::inc()
	{
		return ++value_;
	}

	long atomic_counter::dec()
	{
		return --value_;
	}

	long atomic_counter::get() const
	{
		return value_;
	}


} // booster



