#ifndef CPPCMS_FUNCTION_H
#define CPPCMS_FUNCTION_H

#include "cppcms_error.h"
#include "clone_ptr.h"

namespace cppcms {
	template<typename Type>
	class function;

	class bad_function_call : public cppcms_error {
	public:
		bad_function_call() : 
			cppcms_error("cppcms::bad_function_call")
		{
		}
	};

	#define CPPCMS_FUNCTION							\
	template<typename Result CPPCMS_TEMPLATE_PARAMS >			\
	class function<Result(CPPCMS_TYPE_PARAMS)>				\
	{									\
	public:									\
		typedef Result result_type;					\
		struct callable {						\
			virtual Result call(CPPCMS_TYPE_PARAMS) const=0;	\
			virtual callable *clone() const = 0;			\
			virtual ~callable(){}					\
		};								\
										\
		template<typename R,typename F>					\
		struct callable_impl : public callable {			\
			F func;							\
			callable_impl(F f) : func(f){}				\
			virtual R call(CPPCMS_TYPE_PARAMS) const		\
			{  return func(CPPCMS_CALL_PARAMS); }			\
			virtual callable *clone() const				\
			{ return new callable_impl<R,F>(func); }		\
		};								\
		template<typename F>						\
		struct callable_impl<void,F> : public callable {		\
			F func;							\
			callable_impl(F f) : func(f){}				\
			virtual void call(CPPCMS_TYPE_PARAMS) const		\
			{  func(CPPCMS_CALL_PARAMS); }				\
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
		Result operator()(CPPCMS_TYPE_PARAMS) const			\
		{ 								\
			if(!call_ptr.get()) throw bad_function_call();		\
			return call_ptr->call(CPPCMS_CALL_PARAMS); 		\
		}								\
		bool empty() const { return call_ptr.get()==0; }		\
	private:								\
		util::clone_ptr<callable> call_ptr;				\
	};									\

	#define CPPCMS_TEMPLATE_PARAMS
	#define CPPCMS_TYPE_PARAMS
	#define CPPCMS_CALL_PARAMS
	CPPCMS_FUNCTION
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_TYPE_PARAMS
	#undef CPPCMS_CALL_PARAMS

	#define CPPCMS_TEMPLATE_PARAMS ,typename P1
	#define CPPCMS_TYPE_PARAMS  P1 a1
	#define CPPCMS_CALL_PARAMS a1
	CPPCMS_FUNCTION
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_TYPE_PARAMS
	#undef CPPCMS_CALL_PARAMS

	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2
	#define CPPCMS_TYPE_PARAMS  P1 a1,P2 a2
	#define CPPCMS_CALL_PARAMS a1,a2
	CPPCMS_FUNCTION
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_TYPE_PARAMS
	#undef CPPCMS_CALL_PARAMS
			
	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3
	#define CPPCMS_TYPE_PARAMS  P1 a1,P2 a2,P3 a3
	#define CPPCMS_CALL_PARAMS a1,a2,a3
	CPPCMS_FUNCTION
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_TYPE_PARAMS
	#undef CPPCMS_CALL_PARAMS
	
	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4
	#define CPPCMS_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4
	#define CPPCMS_CALL_PARAMS a1,a2,a3,a4
	CPPCMS_FUNCTION
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_TYPE_PARAMS
	#undef CPPCMS_CALL_PARAMS

	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4,typename P5
	#define CPPCMS_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4,P5 a5
	#define CPPCMS_CALL_PARAMS a1,a2,a3,a4,a5
	CPPCMS_FUNCTION
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_TYPE_PARAMS
	#undef CPPCMS_CALL_PARAMS

	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6
	#define CPPCMS_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4,P5 a5,P6 a6
	#define CPPCMS_CALL_PARAMS a1,a2,a3,a4,a5,a6
	CPPCMS_FUNCTION
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_TYPE_PARAMS
	#undef CPPCMS_CALL_PARAMS
	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7
	#define CPPCMS_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4,P5 a5,P6 a6,P7 a7
	#define CPPCMS_CALL_PARAMS a1,a2,a3,a4,a5,a6,a7
	CPPCMS_FUNCTION
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_TYPE_PARAMS
	#undef CPPCMS_CALL_PARAMS
	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8
	#define CPPCMS_TYPE_PARAMS  P1 a1,P2 a2,P3 a3,P4 a4,P5 a5,P6 a6,P7 a7,P8 a8
	#define CPPCMS_CALL_PARAMS a1,a2,a3,a4,a5,a6,a7,a8
	CPPCMS_FUNCTION
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_TYPE_PARAMS
	#undef CPPCMS_CALL_PARAMS

	#undef CPPCMS_FUNCTION

} // cppcms


#endif
