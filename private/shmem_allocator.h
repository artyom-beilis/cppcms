///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_ALLOCATORS
#define CPPCMS_ALLOCATORS
#include <cppcms/config.h>

#include "boost_interprocess.h"

#include "posix_util.h"
#include <cppcms/cppcms_error.h>
#include <string>

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
};


} // impl
} // cppcms

#endif
