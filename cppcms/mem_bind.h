///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_UTIL_MEM_BIND_H
#define CPPCMS_UTIL_MEM_BIND_H

namespace cppcms { namespace util {

	/// \cond INTERNAL
	namespace details {

		template<typename C,typename P,typename ... P1>
		struct binderX {
			void (C::*member)(P1...);
			P object;
			void operator()(P1... args) const { ((*object).*member)(args...); }
		};
	}

	/// \endcond

	///
	/// Bind a member function \a mem of object referenced by a pointer \a obj creating a functional
	/// object that has an member function void operator()(P1 p) const and calls obj->mem(p) 
	///
	template<typename C,typename P,typename ...P1>
	details::binderX<C,P,P1...> mem_bind(void (C::*mem)(P1...),P obj)
	{
		details::binderX<C,P,P1...> tmp={mem,obj};
		return tmp;
	}


} } // cppcms::util

#endif
