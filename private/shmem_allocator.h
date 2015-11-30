///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_SHMEM_ALLOCATOR_H
#define CPPCMS_IMPL_SHMEM_ALLOCATOR_H
#include <assert.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <cppcms/cstdint.h>
#include <cppcms/config.h>

#include "basic_allocator.h"
#include "posix_util.h"
#include "buddy_allocator.h"
#include <cppcms/cppcms_error.h>

#include <string>
#include <limits>


namespace cppcms {
namespace impl {

class shmem_control : public booster::noncopyable{
public:
	shmem_control(size_t size) :
		size_(size),
		region_(mmap_anonymous(size)),
		memory_(0)
	{
		if(size < sizeof(*memory_))
			throw cppcms_error("shared memory size is too small");
		memory_ = new (region_) cppcms::impl::buddy_allocator(size_);
	}
	~shmem_control()
	{
		memory_->~buddy_allocator();
		::munmap(region_,size_);
	}
	inline size_t size()
	{
		return size_;
	}
	inline size_t available() 
	{ 
		mutex::guard g(lock_);
		return memory_->total_free_memory();

	}
	inline size_t max_available()
	{
		mutex::guard g(lock_);
		return memory_->max_free_chunk();
	}
	inline void *malloc(size_t s)
	{
		mutex::guard g(lock_);
		return memory_->malloc(s);
	}
	inline void free(void *p) 
	{
		mutex::guard g(lock_);
		return memory_->free(p);
	}

private:
	mutex lock_;	
	size_t size_;
	void *region_;
	cppcms::impl::buddy_allocator *memory_;
};

template<typename T,shmem_control *&mm>
class shmem_allocator : public basic_allocator<shmem_allocator<T,mm>, T > {
public :

	typedef basic_allocator<shmem_allocator<T,mm>, T > super;
	template<typename U>
	struct rebind {
		typedef shmem_allocator<U,mm> other;
	};

	template<typename U>
	shmem_allocator (const shmem_allocator< U, mm > &) :
		super()
	{
	}

	shmem_allocator (const shmem_allocator &) :
		super()
	{
	}
	shmem_allocator()
	{
	}

	void *malloc(size_t n) const
	{
		return mm->malloc(n);
	}
	void free(void *p) const
	{
		return mm->free(p);
	}

};


} //
} //


#endif
