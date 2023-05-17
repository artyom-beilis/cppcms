//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_CALLBACK_H
#define BOOSTER_CALLBACK_H

#include <booster/backtrace.h>
#include <memory>
#include <booster/intrusive_ptr.h>
#include <booster/refcounted.h>

namespace booster {
	template<typename Type>
	class callback;

	template<typename Type>
	struct callable;

	///
	/// \brief this exception is thrown in case of calling unassigned/empty
	/// function
	///
	class bad_callback_call : public booster::runtime_error {
	public:
		bad_callback_call() : 
			booster::runtime_error("bad_callback_call")
		{
		}
	};


	template<typename Result, typename ...Params>
	struct callable<Result(Params...)> : public refcounted 
	{
		virtual Result operator()(Params...) = 0;
		virtual ~callable(){}
	};

	///
	/// \brief This is Booster's implementation of std::tr1::callback/booster::callback.
	///
	/// This callback is created from generic object that can be "called" i.e. 
	/// a class with operator() or callback pointer that has same signature as
	/// the callback.
	///
	/// See: http://www.boost.org/doc/html/function.html
	///
	/// Notes:
	///
	/// - this code is not taken from Boost and has slightly different interface.
	/// - as most of compilers do not support Variadic templates yet, this class
	///   is explicitly specialized for Params of size 0 to 8. So maximal amout
	///   of parameters that can be used is 8.
	///
	///
										
	template<typename Result, typename ...Params >			
	class callback<Result(Params...)>			
	{									
	public:									
		typedef Result result_type;
		typedef callable<Result(Params...)> callable_type;						
		///
		/// Pointer to callable object
		///
		/// \ver{v1_2}
		typedef intrusive_ptr<callable_type> pointer_type;		
										
		template<typename R,typename F>					
		struct callable_impl : public callable_type {			
			F func;							
			callable_impl(F f) : func(f){}				
			virtual R operator()(Params... args) 		
			{  return func(args...); }			
		};								
										
		template<typename F>						
		struct callable_impl<void,F> : public callable_type {		
			F func;							
			callable_impl(F f) : func(f){}				
			virtual void operator()(Params... args) 		
			{  func(args...); }				
		};								
										
		///
		/// Default constructor, creates an empty callbacks
		///			
		callback(){}							
										
		template<typename Call>						
		callback(intrusive_ptr<Call> c) : call_ptr(c)			
		{}								
										
		template<typename Call>						
		callback(std::unique_ptr<Call> ptr) : call_ptr(ptr.release())	
		{}								
										
		template<typename Call>						
		callback const &operator=(intrusive_ptr<Call> c)		
		{ call_ptr = c; return *this; }					
										
		template<typename Call>						
		callback const &operator=(std::unique_ptr<Call> c)		
		{ call_ptr = 0; call_ptr = c.release(); return *this; }		
										
		template<typename F>						
		callback(F func) : call_ptr(new callable_impl<Result,F>(func)) 	
		{}								
										
		callback(callback const &other) : call_ptr(other.call_ptr) {}	

		callback(callback &&other) : call_ptr(std::move(other.call_ptr))
		{
		}
										
		template<typename F>						
		callback const &operator=(F func)				
		{ 								
			call_ptr = new callable_impl<Result,F>(func);		
			return *this;						
		}								
										
		callback &operator=(callback &&other)
		{
			call_ptr = std::move(other.call_ptr);
			return *this;
		}
		callback const &operator=(callback const &other)		
		{ 								
			if(this != &other) { call_ptr=other.call_ptr; } 	
			return *this;						
		}								
										
		Result operator()(Params ...args) const			
		{ 								
			if(!call_ptr.get()) throw bad_callback_call();		
			return (*call_ptr)(args...); 		
		}								
										
		///
		/// Return true if the callback is empty
		///
		bool empty() const { return call_ptr.get()==0; }		
										
		///
		/// Returns true if the callback is not empty
		///
		operator bool() const { return !empty(); }			
										
		
		void swap(callback &other) { call_ptr.swap(other.call_ptr); }	
		
		pointer_type const &get_pointer() const { return call_ptr; }	
		pointer_type &get_pointer() { return call_ptr; }		
										
	private:								
		pointer_type call_ptr;						
	};									


} // booster


#endif
