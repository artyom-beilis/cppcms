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
#ifndef CPPCMS_UTIL_CALLBACK0_H
#define CPPCMS_UTIL_CALLBACK0_H
#include "clone_ptr.h"

namespace cppcms { namespace util {

///
/// \brief Function object, similar to C++0x std::function<void()>, or boost::function<void()> 
///
/// Callback object, it can be created with any "function like object" -- a class with operator() or C function
/// with appropriate signature.
///

class callback0 {

	struct callable {
		virtual void call() = 0;
		virtual callable *clone() const = 0;
		virtual ~callable() {};
	};

	template<typename T>
	struct callable_functor : public callable{
		T func;
		callable_functor(T f) : func(f) {}
		virtual ~callable_functor() { }
		virtual void call() { func(); }
		virtual callable *clone() const { return new callable_functor<T>(func); }
	};

	clone_ptr<callable> call_ptr;
public:
	typedef void result_type;

	///
	/// Call the assigned function, does nothing if function was not assigned
	///
	void operator()() const
	{
		if(call_ptr.get()) {
			call_ptr->call();
		}
	}
	
	///
	/// Create an empty callback
	///
	callback0(){}


	///
	/// Copy constructor
	///
	
	callback0(callback0 const &other) : call_ptr(other.call_ptr)
	{
	}

	///
	/// Assignment operator
	///

	callback0 const &operator=(callback0 const &other)
	{
		if(this!=&other)
			call_ptr = other.call_ptr;
		return *this;
	}
	
	///
	/// Create a callback and copy callable object T to it.
	///
	template<typename T>
	callback0(T c) : call_ptr(new callable_functor<T>(c)) 
	{
	}
	
	///
	/// Assign a callable object to it
	///
	template<typename T>
	callback0 const &operator=(T c)
	{
		call_ptr.reset(new callable_functor<T>(c));
		return *this;
	}

	///
	/// Swap two callbacks
	///
	void swap(callback0 &other)
	{
		call_ptr.swap(other.call_ptr);
	}

};



}} // cppcms util


#endif
