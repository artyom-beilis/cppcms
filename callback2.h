#ifndef CPPCMS_UTIL_CALLBACK2_H
#define CPPCMS_UTIL_CALLBACK2_H
#include "clone_ptr.h"

namespace cppcms { namespace util {

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

	void operator()(P1 p1,P2 p2) const
	{
		if(call_ptr.get()) {
			call_ptr->call(p1,p2);
		}
	}

	callback2(){}

	template<typename T>
	callback2(T c) : call_ptr(new callable_functor<T>(c)) 
	{
	}
	
	template<typename T>
	callback2 const &operator=(T c)
	{
		call_ptr.reset(new callable_functor<T>(c));
		return *this;
	}

};



}} // cppcms util


#endif
