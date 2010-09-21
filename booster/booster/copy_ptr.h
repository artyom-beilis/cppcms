//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_UTIL_COPY_PTR_H
#define BOOSTER_UTIL_COPY_PTR_H

namespace booster { 

	///
	/// \brief a smart pointer similar to std::auto_ptr but it copies
	///   underlying object on pointer copy instead of moving its ownership.
	///
	/// Note: Underlying object has same constness as the pointer itself (not like in ordinary pointer).
	///
	/// Don't use it with polymorphic classes. Prefer clone_ptr instead.
	///
	template<typename T>
	class copy_ptr {
		T *ptr_;
	public:
		copy_ptr() : ptr_(0) {}
		copy_ptr(T *v) : ptr_(v) {}
		copy_ptr(copy_ptr const &other) :
			ptr_(other.ptr_ ? new T(*other.ptr_) : 0)
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
					ptr_=new T(*other.ptr_);
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

} // booster

#endif
