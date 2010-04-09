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
#ifndef CPPCMS_UTIL_CLONE_PTR_H
#define CPPCMS_UTIL_CLONE_PTR_H

namespace cppcms { namespace util {

	
	///
	/// \brief a smart pointer similar to std::auto_ptr but it clones (by calling T::clone())
	///   underlying object on copy instead of moving its ownership.
	///
	template<typename T>
	class clone_ptr {
		T *ptr_;
	public:
		clone_ptr() : ptr_(0) {}
		clone_ptr(T *v) : ptr_(v) {}
		clone_ptr(clone_ptr const &other) : ptr_(0)
		{
			if(other.ptr_)
				ptr_=other.ptr_->clone();
		}
		clone_ptr const &operator=(clone_ptr const &other)
		{
			if(this != &other) {
				if(ptr_) {
					delete ptr_;
					ptr_=0;
				}
				if(other.ptr_) {
					ptr_=other.ptr_->clone();
				}
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

}} // cppcms::util

#endif
