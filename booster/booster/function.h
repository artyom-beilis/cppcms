//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_FUNCTION_H
#define BOOSTER_FUNCTION_H

#include <stdexcept>
#include <booster/clone_ptr.h>

namespace booster {
	template<typename Type>
	class function;

	class bad_function_call : public std::runtime_error {
	public:
		bad_function_call() : 
			std::runtime_error("bad_function_call")
		{
		}
	};

	#define BOOSTER_FUNCTION							\
	template<typename Result BOOSTER_TEMPLATE_PARAMS >			\
	class function<Result(BOOSTER_TYPE_PARAMS)>				\
	{									\
	public:									\
		typedef Result result_type;					\
		struct callable {						\
			virtual Result call(BOOSTER_TYPE_PARAMS) =0;		\
			virtual callable *clone() const = 0;			\
			virtual ~callable(){}					\
		};								\
										\
		template<typename R,typename F>					\
		struct callable_impl : public callable {			\
			F func;							\
			callable_impl(F f) : func(f){}				\
			virtual R call(BOOSTER_TYPE_PARAMS) 			\
			{  return func(BOOSTER_CALL_PARAMS); }			\
			virtual callable *clone() const				\
			{ return new callable_impl<R,F>(func); }		\
		};								\
		template<typename F>						\
		struct callable_impl<void,F> : public callable {		\
			F func;							\
			callable_impl(F f) : func(f){}				\
			virtual void call(BOOSTER_TYPE_PARAMS) 			\
			{  func(BOOSTER_CALL_PARAMS); }				\
			virtual callable *clone() const				\
			{ return new callable_impl<void,F>(func); }		\
		};								\
		function(){}							\
		template<typename F>						\
		function(F func) : call_ptr(new callable_impl<Result,F>(func)) 	\
		{}								\
		function(function const &other) : call_ptr(other.call_ptr) {}	\
		function const &operator=(function const &other)		\
		{ 								\
			if(this != &other) { call_ptr=other.call_ptr; } 	\
			return *this;						\
		}								\
		Result operator()(BOOSTER_TYPE_PARAMS) const			\
		{ 								\
			if(!call_ptr.get()) throw bad_function_call();		\
			return call_ptr->call(BOOSTER_CALL_PARAMS); 		\
		}								\
		bool empty() const { return call_ptr.get()==0; }		\
		void swap(function &other) { call_ptr.swap(other.call_ptr); }	\
	private:								\
		clone_ptr<callable> call_ptr;					\
	};									\

	#define BOOSTER_TEMPLATE_PARAMS
	#define BOOSTER_TYPE_PARAMS
	#define BOOSTER_CALL_PARAMS
	BOOSTER_FUNCTION
	#undef BOOSTER_TEMPLATE_PARAMS
	#undef BOOSTER_TYPE_PARAMS
	#undef BOOSTER_CALL_PARAMS

	#define BOOSTER_TEMPLATE_PARAMS ,typename P1
	#define BOOSTER_TYPE_PARAMS  P1 a1
	#define BOOSTER_CALL_PARAMS a1
	BOOSTER_FUNCTION
	#undef BOOSTER_TEMPLATE_PARAMS
	#undef BOOSTER_TYPE_PARAMS
	#undef BOOSTER_CALL_PARAMS

	#define BOOSTER_TEMPLATE_PARAMS ,typename P1,typename P2
	#define BOOSTER_TYPE_PARAMS  P1 a1,P2 a2
	#define BOOSTER_CALL_PARAMS a1,a2
	BOOSTER_FUNCTION
	#undef BOOSTER_TEMPLATE_PARAMS
	#undef BOOSTER_TYPE_PARAMS
	#undef BOOSTER_CALL_PARAMS
			
	#define BOOSTER_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3
	#define BOOSTER_TYPE_PARAMS  P1 a1,P2 a2,P3 a3
	#define BOOSTER_CALL_PARAMS a1,a2,a3
	BOOSTER_FUNCTION
	#undef BOOSTER_TEMPLATE_PARAMS
	#undef BOOSTER_TYPE_PARAMS
	#undef BOOSTER_CALL_PARAMS
	
	#define BOOSTER_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4
	#define BOOSTER_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4
	#define BOOSTER_CALL_PARAMS a1,a2,a3,a4
	BOOSTER_FUNCTION
	#undef BOOSTER_TEMPLATE_PARAMS
	#undef BOOSTER_TYPE_PARAMS
	#undef BOOSTER_CALL_PARAMS

	#define BOOSTER_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4,typename P5
	#define BOOSTER_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4,P5 a5
	#define BOOSTER_CALL_PARAMS a1,a2,a3,a4,a5
	BOOSTER_FUNCTION
	#undef BOOSTER_TEMPLATE_PARAMS
	#undef BOOSTER_TYPE_PARAMS
	#undef BOOSTER_CALL_PARAMS

	#define BOOSTER_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6
	#define BOOSTER_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4,P5 a5,P6 a6
	#define BOOSTER_CALL_PARAMS a1,a2,a3,a4,a5,a6
	BOOSTER_FUNCTION
	#undef BOOSTER_TEMPLATE_PARAMS
	#undef BOOSTER_TYPE_PARAMS
	#undef BOOSTER_CALL_PARAMS
	#define BOOSTER_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7
	#define BOOSTER_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4,P5 a5,P6 a6,P7 a7
	#define BOOSTER_CALL_PARAMS a1,a2,a3,a4,a5,a6,a7
	BOOSTER_FUNCTION
	#undef BOOSTER_TEMPLATE_PARAMS
	#undef BOOSTER_TYPE_PARAMS
	#undef BOOSTER_CALL_PARAMS
	#define BOOSTER_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8
	#define BOOSTER_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4,P5 a5,P6 a6,P7 a7,P8 a8
	#define BOOSTER_CALL_PARAMS a1,a2,a3,a4,a5,a6,a7,a8
	BOOSTER_FUNCTION
	#undef BOOSTER_TEMPLATE_PARAMS
	#undef BOOSTER_TYPE_PARAMS
	#undef BOOSTER_CALL_PARAMS

	#undef BOOSTER_FUNCTION

} // booster


#endif
