//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_HOLD_PTR_H
#define BOOSTER_HOLD_PTR_H

namespace booster { 

	///
	/// \brief a smart pointer similar to std::unique_ptr but it is non-copyable and
	/// underlying object has same constness as the pointer itself (not like in ordinary pointer).
	///
	template<typename T>
	class hold_ptr {
		T *ptr_;
	public:
		hold_ptr(hold_ptr const &other) = delete; // non copyable 
		hold_ptr const &operator=(hold_ptr const &other) = delete; // non assignable
		hold_ptr() : ptr_(0) {}
		explicit hold_ptr(T *v) : ptr_(v) {}
		hold_ptr(hold_ptr &&other) : ptr_(other.ptr_)
		{
			other.ptr_ = 0;
		}
		hold_ptr &operator=(hold_ptr &&other)
		{
			if(this!=&other) {
				this->swap(other);
				other.reset();
			}
			return *this;
		}
		~hold_ptr() 
		{
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
		void swap(hold_ptr &other)
		{
			T *tmp=other.ptr_;
			other.ptr_=ptr_;
			ptr_=tmp;
		}
	};
} // booster

#endif
