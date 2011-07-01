//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_ITERATOR_ITERATOR_FACADE_H
#define BOOSTER_ITERATOR_ITERATOR_FACADE_H

#include <cstddef>

namespace booster { 

	struct bidirectional_traversal_tag {};

	template<
		typename Derived,
		typename Value,
		typename Category,
		typename Reference = Value &,
		typename Difference = std::ptrdiff_t
	>
	class iterator_facade;

	namespace details {
		template<typename T>
		struct reference_to_pointer;

		template<typename R>
		struct reference_to_pointer<R &>
		{
			typedef R *pointer;
		};

		template<typename R>
		struct reference_to_pointer<R const &>
		{
			typedef R const *pointer;
		};
	}
	
	template<
		typename Derived,
		typename Value,
		typename Reference,
		typename Difference
	>
	class iterator_facade<Derived,Value,bidirectional_traversal_tag,Reference,Difference> :
		public std::iterator<std::bidirectional_iterator_tag,Value> 
	{
	public:
		typedef Value value_type;
		typedef Reference reference;
		typedef typename details::reference_to_pointer<reference>::pointer pointer;
		typedef Difference difference_type;
		typedef std::bidirectional_iterator_tag iterator_category;

		reference operator*() const
		{
			return self()->dereference();
		}
		pointer operator->() const
		{
			return &self()->dereference();
		}
		Derived &operator++()
		{
			self()->increment();
			return *self();
		}
		Derived &operator--()
		{
			self()->decrement();
			return *self();
		}
		Derived operator++(int)
		{
			Derived d(*self());
			self()->increment();
			return d;
		}
		Derived operator--(int)
		{
			Derived d(*self());
			self()->decrement();
			return d;
		}
		bool operator==(Derived const &other) const
		{
			return self()->equal(other);
		}
		bool operator!=(Derived const &other) const
		{
			return !self()->equal(other);
		}

	private:
		Derived const *self() const
		{
			return static_cast<Derived const *>(this);
		}
		Derived *self()
		{
			return static_cast<Derived *>(this);
		}
	};


} // booster

#endif
