#ifndef CPPCMS_UTIL_CALLBACK2_H
#define CPPCMS_UTIL_CALLBACK2_H
#include "clone_ptr.h"

namespace cppcms { namespace util {

///
/// \brief Function object, similar to C++0x std::function<void(P1,P2)>, or boost::function<void(P1,P2)> 
///
/// Callback object, it can be created with any "function like object" -- a class with operator()(P1,P2) or C function
/// with appropriate signature.
///

template<typename P1,typename P2>
class callback2 {

	struct callable {
		virtual void call(P1,P2) = 0;
		virtual callable *clone() const = 0;
		virtual ~callable() {};
	};

	template<typename T>
	struct callable_functor : public callable{
		T func;
		callable_functor(T f) : func(f) {}
		virtual ~callable_functor() { }
		virtual void call(P1 p1,P2 p2) { func(p1,p2); }
		virtual callable *clone() const { return new callable_functor<T>(func); }
	};

	clone_ptr<callable> call_ptr;
public:
	typedef void result_type;
	typedef P1 first_argument_type;
	typedef P2 second_argument_type;

	///
	/// Call the assigned function, does nothing if function was not assigned
	///
	void operator()(P1 p1,P2 p2) const
	{
		if(call_ptr.get()) {
			call_ptr->call(p1,p2);
		}
	}

	///
	/// Create an empty callback
	///
	callback2(){}
	///
	/// Copy constructor
	///
	
	callback2(callback2 const &other) : call_ptr(other.call_ptr)
	{
	}

	///
	/// Assignment operator
	///

	callback2 const &operator=(callback2 const &other)
	{
		if(this!=&other)
			call_ptr = other.call_ptr;
		return *this;
	}

	///
	/// Create a callback and copy callable object T to it.
	///
	template<typename T>
	callback2(T c) : call_ptr(new callable_functor<T>(c)) 
	{
	}
	
	///
	/// Assign a callable object to it
	///
	template<typename T>
	callback2 const &operator=(T c)
	{
		call_ptr.reset(new callable_functor<T>(c));
		return *this;
	}
	///
	/// Swap two callbacks
	///
	void swap(callback2 &other)
	{
		call_ptr.swap(other.call_ptr);
	}

};



}} // cppcms util


#endif
