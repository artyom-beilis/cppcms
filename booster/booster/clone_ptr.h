//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_CLONE_PTR_H
#define BOOSTER_CLONE_PTR_H

namespace booster {
	
	///
	/// \brief a smart pointer similar to std::auto_ptr but it clones (by calling T::clone())
	///   underlying object on copy instead of moving its ownership.
	///
	template<typename T>
	class clone_ptr {
		T *ptr_;
	public:
		clone_ptr() : ptr_(0) {}
		explicit clone_ptr(T *v) : ptr_(v) {}
		clone_ptr(clone_ptr const &other) : ptr_(0)
		{
			if(other.ptr_)
				ptr_=other.ptr_->clone();
		}
		clone_ptr const &operator=(clone_ptr const &other)
		{
			if(this != &other) {
				clone_ptr tmp(other);
				swap(tmp);
			}
			return *this;
		}
		~clone_ptr() {
			if(ptr_) delete ptr_;
		}

		T *get() const { return ptr_; }
		T &operator *() const { return *ptr_; }
		T *operator->() const { return ptr_; }

		T *release() { T *tmp=ptr_; ptr_=0; return tmp; }
		void reset(T *p=0)
		{
			if(ptr_) delete ptr_;
			ptr_=p;
		}
		void swap(clone_ptr &other)
		{
			T *tmp=other.ptr_;
			other.ptr_=ptr_;
			ptr_=tmp;
		}
	};

} // booster

#endif
