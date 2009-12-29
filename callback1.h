#ifndef CPPCMS_UTIL_CALLBACK1_H
#define CPPCMS_UTIL_CALLBACK1_H
#include "clone_ptr.h"

namespace cppcms { namespace util {

///
/// \brief Function object, similar to C++0x std::function<void(P1)>, or boost::function<void(P1)> 
///
/// Callback object, it can be created with any "function like object" -- a class with operator()(P1) or C function
/// with appropriate signature.
///

template<typename P1>
class callback1 {

	struct callable {
		virtual void call(P1) = 0;
		virtual callable *clone() const = 0;
		virtual ~callable() {};
	};

	template<typename T>
	struct callable_functor : public callable{
		T func;
		callable_functor(T f) : func(f) {}
		virtual ~callable_functor() { }
		virtual void call(P1 p1) { func(p1); }
		virtual callable *clone() const { return new callable_functor<T>(func); }
	};

	clone_ptr<callable> call_ptr;
public:
	typedef void result_type;
	typedef P1 argument_type;

	///
	/// Call the assigned function, does nothing if function was not assigned
	///
	void operator()(P1 p1) const
	{
		if(call_ptr.get()) {
			call_ptr->call(p1);
		}
	}

	///
	/// Create an empty callback
	///
	callback1(){}

	///
	/// Create a callback and copy callable object T to it.
	///
	template<typename T>
	callback1(T c) : call_ptr(new callable_functor<T>(c)) 
	{
	}
	
	///
	/// Assign a callable object to it
	///
	template<typename T>
	callback1 const &operator=(T c)
	{
		call_ptr.reset(new callable_functor<T>(c));
		return *this;
	}
	
	///
	/// Swap two callbacks
	///
	void swap(callback1 &other)
	{
		call_ptr.swap(other.call_ptr);
	}

};



}} // cppcms util


#endif
