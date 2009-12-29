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
