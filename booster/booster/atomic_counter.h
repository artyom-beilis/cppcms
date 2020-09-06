//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_ATOMIC_COUNT_H
#define BOOSTER_ATOMIC_COUNT_H

#include <booster/config.h>
#include <atomic>


///
/// \brief Booster library namespace. The library that implements Boost Like API
/// in ABI backward compatible way
///
namespace booster {

	///
	/// \brief Atomic counter is a class that allows perform counting in thread safe way.
	///

	class BOOSTER_API atomic_counter {
	public:
		///
		/// Create a counter with initial value v
		///
    		explicit atomic_counter( long v );
		~atomic_counter();

		///
		/// Increment and return the result after increment atomically
		///
    		long operator++()
		{
			return inc();
		}
		///
		/// Decrement and return the result after decrement atomically
		///
    		long operator--()
		{
			return dec();
		}
		///
		/// Return current value - atomically
		///
		operator long() const 
		{
			return get();
		}
	private:
		long inc();
		long dec();
		long get() const;

		atomic_counter(atomic_counter const &) = delete;
		atomic_counter & operator=(atomic_counter const &) = delete;

		std::atomic_long value_;
	};

} // booster

#endif


