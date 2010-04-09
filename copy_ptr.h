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
#ifndef CPPCMS_UTIL_COPY_PTR_H
#define CPPCMS_UTIL_COPY_PTR_H

namespace cppcms { namespace util {

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

}} // cppcms::util

#endif
