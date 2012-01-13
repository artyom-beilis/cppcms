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
class shmem_allocator {
public :
	typedef T value_type;
	typedef T *pointer;
	typedef T &reference;
	typedef const T &const_reference;
	typedef const T *const_pointer;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template<typename U>
	struct rebind {
		typedef shmem_allocator<U,mm> other;
	};


	template<typename U> shmem_allocator (const shmem_allocator< U, mm > &) { };

	shmem_allocator (const shmem_allocator &){ };
	shmem_allocator() {};

	pointer allocate(size_type cnt, std::allocator<void>::const_pointer = 0) const
	{
		void *memory=mm->malloc(cnt*sizeof(T));
		if(!memory) {
			throw std::bad_alloc();
		}
		return (pointer)memory;
	};
	void deallocate(pointer p, size_type) const
	{
		mm->free(p);
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

	bool operator==(shmem_allocator const&) const 
	{
		return true;
	}
	bool operator!=(shmem_allocator const& a) const 
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
