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
#ifndef CPPCMS_UTIL_HOLD_PTR_H
#define CPPCMS_UTIL_HOLD_PTR_H

namespace cppcms { namespace util {

	///
	/// \brief a smart pointer similar to std::auto_ptr but it is non-copyable and
	/// underlying object has same constness as the pointer itself (not like in ordinary pointer).
	///
	template<typename T>
	class hold_ptr {
		T *ptr_;
		hold_ptr(hold_ptr const &other); // non copyable 
		hold_ptr const &operator=(hold_ptr const &other); // non assignable
	public:
		hold_ptr() : ptr_(0) {}
		hold_ptr(T *v) : ptr_(v) {}
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
} } // cppcms::util;

#endif
