///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>
//
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_BASIC_ALLOCATOR_H
#define CPPCMS_IMPL_BASIC_ALLOCATOR_H

#include <limits>

namespace cppcms {
namespace impl {

template<typename Base,typename T>
class basic_allocator {
public :

	Base &self()
	{
		return static_cast<Base &>(*this);
	}
	Base const &self() const
	{
		return static_cast<Base const &>(*this);
	}

	typedef T value_type;
	typedef T *pointer;
	typedef T &reference;
	typedef const T &const_reference;
	typedef const T *const_pointer;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	basic_allocator() {}

	pointer allocate(size_type cnt, std::allocator<void>::const_pointer = 0) const
	{
		void *memory=self().malloc(cnt*sizeof(T));
		if(!memory) {
			throw std::bad_alloc();
		}
		return (pointer)memory;
	};
	void deallocate(pointer p, size_type) const
	{
		self().free(p);
	};
	void construct(pointer p, const T& t) const
	{
		new(p) T(t);
	}
	void destroy(pointer p) const
	{
		p->~T();
	}
	pointer address(reference x) const
	{
		return &x;
	}
	const_pointer address(const_reference x) const
	{
		return &x;
	}

	bool operator==(basic_allocator const&) const
	{
		return true;
	}
	bool operator!=(basic_allocator const& a) const
	{
		return false;
	}

	size_type max_size() const throw()
	{
		return std::numeric_limits<size_t>::max();
	}
};

} // impl
} // cppcms



#endif
