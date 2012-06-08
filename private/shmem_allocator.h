///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_ALLOCATORS
#define CPPCMS_ALLOCATORS
#include <cppcms/config.h>

#include "basic_allocator.h"
#include "boost_interprocess.h"

#include "posix_util.h"
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
		memory_(boost::interprocess::create_only,region_,size_)
	{
	}
	~shmem_control()
	{
		::munmap(region_,size_);
	}
	inline size_t size()
	{
		return size_;
	}
	inline size_t available() 
	{ 
		mutex::guard g(lock_);
		return memory_.get_free_memory();

	}
	inline void *malloc(size_t s)
	{
		mutex::guard g(lock_);
		try {
			return memory_.allocate(s);
		}
		catch(boost::interprocess::bad_alloc const &/*alloc*/)
		{
			return 0;
		}
	}
	inline void free(void *p) 
	{
		mutex::guard g(lock_);
		return memory_.deallocate(p);
	}
private:
	mutex lock_;	
	size_t size_;
	void *region_;
	boost::interprocess::managed_external_buffer memory_;
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


} // impl
} // cppcms

#endif
