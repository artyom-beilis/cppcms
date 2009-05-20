#ifndef CPPCMS_UTIL_CALLBACK4_H
#define CPPCMS_UTIL_CALLBACK4_H
#include "clone_ptr.h"

namespace cppcms { namespace util {

template<typename P1,typename P2,typename P3,typename P4>
class callback4 {

	struct callable {
		virtual void call(P1,P2,P3,P4) = 0;
		virtual callable *clone() const = 0;
		virtual ~callable() {};
	};

	template<typename T>
	struct callable_functor : public callable{
		T func;
		callable_functor(T f) : func(f) {}
		virtual ~callable_functor() { }
		virtual void call(P1 p1,P2 p2,P3 p3,P4 p4) { func(p1,p2,p3,p4); }
		virtual callable *clone() const { return new callable_functor<T>(func); }
	};

	clone_ptr<callable> call_ptr;
public:
	typedef void result_type;

	void operator()(P1 p1,P2 p2,P3 p3,P4 p4) const
	{
		if(call_ptr.get()) {
			call_ptr->call(p1,p2,p3,p4);
		}
	}

	callback4(){}

	template<typename T>
	callback4(T c) : call_ptr(new callable_functor<T>(c)) 
	{
	}
	
	template<typename T>
	callback4 const &operator=(T c)
	{
		call_ptr.reset(new callable_functor<T>(c));
		return *this;
	}

	void swap(callback4 &other)
	{
		call_ptr.swap(other.call_ptr);
	}
};



}} // cppcms util


#endif
