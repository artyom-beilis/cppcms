#ifndef CPPCMS_UTIL_HOLD_PTR_H
#define CPPCMS_UTIL_HOLD_PTR_H

namespace cppcms { namespace util {

	// Just non-copyable auto_ptr
	template<typename T>
	class hold_ptr {
		T *ptr_;
		copy_ptr(copy_ptr const &other); // non copyable 
		copy_ptr const &operator=(copy_ptr const &other); // non assignable
	public:
		hold_ptr() : ptr_(0) {}
		hold_ptr(T *v) : ptr_v() {}
			ptr_(other.ptr ? new T(*other.ptr) : 0)
		{
		}
		~hold_ptr() 
		{
			if(ptr_) delete ptr_;
		}
		T const &operator *() const { return *ptr_; }
		T &operator *() { return *ptr_; }
		T const *operator->() const { return ptr_; }
		T *operator->() { return ptr_; }
		T *release() { T *tmp=ptr_; ptr_=0; return tmp; }
		void reset(T *p=0)
		{
			if(ptr_) delete ptr_;
			ptr_=p;
		}
	};
} } // cppcms::util;

#endif
