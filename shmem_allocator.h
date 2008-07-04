#ifndef CPPCMS_ALLOCATORS
#define CPPCMS_ALLOCATORS
#include "cppcms_error.h"
#include <mm.h>
#include <string>
namespace cppcms {

class shmem_control {
	MM *memory;
public:
	shmem_control(size_t size,char const *file) {
		if((memory=mm_create(size,file))==NULL) {
			std::string e(mm_error());
			throw cppcms_error("Failed to create shared memory "+e);
		}
	};

	~shmem_control() {
		mm_destroy(memory);
	};
	inline MM *get() const { return memory; };
	inline size_t available() const { return mm_available(memory); };
	inline void *malloc(size_t s) const{ return mm_malloc(memory,s); };
	inline void free(void *p) const {  mm_free(memory,p); };
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
		void *memory=mm->malloc(cnt*sizeof(T));
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
