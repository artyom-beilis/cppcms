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
#ifndef CPPCMS_UTIL_CALLBACK3_H
#define CPPCMS_UTIL_CALLBACK3_H
#include "clone_ptr.h"

namespace cppcms { namespace util {

///
/// \brief Function object, similar to C++0x std::function<void(P1,P2,P3)>, or boost::function<void(P1,P2,P3)> 
///
/// Callback object, it can be created with any "function like object" -- a class with operator()(P1,P2,P3) or C function
/// with appropriate signature.
///

template<typename P1,typename P2,typename P3>
class callback3 {

	struct callable {
		virtual void call(P1,P2,P3) = 0;
		virtual callable *clone() const = 0;
		virtual ~callable() {};
	};

	template<typename T>
	struct callable_functor : public callable{
		T func;
		callable_functor(T f) : func(f) {}
		virtual ~callable_functor() { }
		virtual void call(P1 p1,P2 p2,P3 p3) { func(p1,p2,p3); }
		virtual callable *clone() const { return new callable_functor<T>(func); }
	};

	clone_ptr<callable> call_ptr;
public:
	typedef void result_type;

	///
	/// Call the assigned function, does nothing if function was not assigned
	///
	void operator()(P1 p1,P2 p2,P3 p3) const
	{
		if(call_ptr.get()) {
			call_ptr->call(p1,p2,p3);
		}
	}

	///
	/// Create an empty callback
	///
	callback3(){}
	///
	/// Copy constructor
	///
	
	callback3(callback3 const &other) : call_ptr(other.call_ptr)
	{
	}

	///
	/// Assignment operator
	///

	callback3 const &operator=(callback3 const &other)
	{
		if(this!=&other)
			call_ptr = other.call_ptr;
		return *this;
	}

	///
	/// Create a callback and copy callable object T to it.
	///
	template<typename T>
	callback3(T c) : call_ptr(new callable_functor<T>(c)) 
	{
	}
	
	///
	/// Assign a callable object to it
	///
	template<typename T>
	callback3 const &operator=(T c)
	{
		call_ptr.reset(new callable_functor<T>(c));
		return *this;
	}

	///
	/// Swap two callbacks
	///
	void swap(callback3 &other)
	{
		call_ptr.swap(other.call_ptr);
	}
};



}} // cppcms util


#endif
