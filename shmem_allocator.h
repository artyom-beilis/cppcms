#ifndef CPPCMS_ALLOCATORS
#define CPPCMS_ALLOCATORS
#include "config.h"

#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/interprocess/managed_external_buffer.hpp>
#else
#   include <cppcms_boost/interprocess/managed_external_buffer.hpp>
    namespace boost = cppcms_boost;
#endif

#include "posix_util.h"
#include "cppcms_error.h"
#include <string>

namespace cppcms {
namespace impl {

class shmem_control : public util::noncopyable{
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
	inline size_t available() 
	{ 
		mutex::guard g(lock_);
		return memory_.get_free_memory();

	}
	inline void *malloc(size_t s)
	{
		mutex::guard g(lock_);
		return memory_.allocate(s);
	}
	inline void free(void *p) 
	{
		mutex::guard g(lock_);
		return memory_.deallocate(p);
	}
private:
	process_shared_mutex lock_;	
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


	template<typename U> shmem_allocator (const shmem_allocator< U, mm > &a) { };
	shmem_allocator (const shmem_allocator &__a){ };
	shmem_allocator() {};

	inline pointer allocate(size_type cnt, std::allocator<void>::const_pointer = 0) const
	{
		return void *memory=mm->malloc(cnt*sizeof(T));
		if(!memory) {
			throw std::bad_alloc();
		}
		return (pointer)memory;
	};
	inline void deallocate(pointer p, size_type) const
	{
		mm->free(p);
	};
	inline void construct(pointer p, const T& t) const { new(p) T(t); }
	inline void destroy(pointer p) const { p->~T(); }

	inline bool operator==(shmem_allocator const&) const { return true; }
	inline bool operator!=(shmem_allocator const& a) const { return false; }
};


};

#endif
