#ifndef CPPCMS_UTIL_COPY_PTR_H
#define CPPCMS_UTIL_COPY_PTR_H

namespace cppcms { namespace util {

	// Similar to auto_ptr, but actually creates
	// copy of target instead moving it
	template<typename T>
	class copy_ptr {
		T *ptr_;
	public:
		copy_ptr() : ptr_(0) {}
		copy_ptr(T *v) : ptr_v() {}
		copy_ptr(copy_ptr const &other) :
			ptr_(other.ptr ? new T(*other.ptr) : 0)
		{
		}
		copy_ptr const &operator=(copy_ptr const &other)
		{
			if(this != &other) {
				if(ptr_) {
					delete ptr_;
					ptr_=0;
				}
				if(other.ptr_) {
					ptr_=new T(*other.ptr);
				}
			}
			return *this;
		}
		~copy_ptr() {
			if(ptr_) delete ptr_;
		}

		T const *get() const { return ptr_; }
		T *get() { return ptr_; }

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
		void swap(copy_ptr &other)
		{
			T *tmp=other.ptr_;
			other.ptr_=ptr_;
			ptr_=tmp;
		}
	};

}} // cppcms::util

#endif
