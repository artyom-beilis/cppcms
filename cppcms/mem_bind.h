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
		template<typename C,typename P>
		struct binder0 {
			void (C::*member)();
			P object;
			void operator()() const { ((*object).*member)(); }
		};
		template<typename C,typename P,typename P1>
		struct binder1 {
			void (C::*member)(P1);
			P object;
			void operator()(P1 p1) const { ((*object).*member)(p1); }
		};
		template<typename C,typename P,typename P1,typename P2>
		struct binder2 {
			void (C::*member)(P1,P2);
			P object;
			void operator()(P1 p1,P2 p2) const { ((*object).*member)(p1,p2); }
		};
		template<typename C,typename P,typename P1,typename P2,typename P3>
		struct binder3 {
			void (C::*member)(P1,P2,P3);
			P object;
			void operator()(P1 p1,P2 p2,P3 p3) const { ((*object).*member)(p1,p2,p3); }
		};
		template<typename C,typename P,typename P1,typename P2,typename P3,typename P4>
		struct binder4 {
			void (C::*member)(P1,P2,P3,P4);
			P object;
			void operator()(P1 p1,P2 p2,P3 p3,P4 p4) const { ((*object).*member)(p1,p2,p3,p4); }
		};
	}

	/// \endcond

	///
	/// Bind a member function \a mem of object referenced by a pointer \a obj creating a functional
	/// object that has an member function void operator()() const and calls obj->mem() 
	///
	template<typename C,typename P>
	details::binder0<C,P> mem_bind(void (C::*mem)(),P obj)
	{
		details::binder0<C,P> tmp={mem,obj};
		return tmp;
	}
	///
	/// Bind a member function \a mem of object referenced by a pointer \a obj creating a functional
	/// object that has an member function void operator()(P1 p) const and calls obj->mem(p) 
	///
	template<typename C,typename P,typename P1>
	details::binder1<C,P,P1> mem_bind(void (C::*mem)(P1),P obj)
	{
		details::binder1<C,P,P1> tmp={mem,obj};
		return tmp;
	}
	///
	/// Bind a member function \a mem of object referenced by a pointer \a obj creating a functional
	/// object that has an member function void operator()(P1 p1,P2 p2) const and calls obj->mem(p1,p2) 
	///
	template<typename C,typename P,typename P1,typename P2>
	details::binder2<C,P,P1,P2> mem_bind(void (C::*mem)(P1,P2),P obj)
	{
		details::binder2<C,P,P1,P2> tmp={mem,obj};
		return tmp;
	}
	///
	/// Bind a member function \a mem of object referenced by a pointer \a obj creating a functional
	/// object that has an member function void operator()(P1 p1,P2 p2,P3 p3) const and calls obj->mem(p1,p2,p3) 
	///
	template<typename C,typename P,typename P1,typename P2,typename P3>
	details::binder3<C,P,P1,P2,P3> mem_bind(void (C::*mem)(P1,P2,P3),P obj)
	{
		details::binder3<C,P,P1,P2,P3> tmp={mem,obj};
		return tmp;
	}
	///
	/// Bind a member function \a mem of object referenced by a pointer \a obj creating a functional
	/// object that has an member function void operator()(P1 p1,P2 p2,P3 p3,P4 ) const and calls obj->mem(p1,p2,p3,p4) 
	///
	template<typename C,typename P,typename P1,typename P2,typename P3,typename P4>
	details::binder4<C,P,P1,P2,P3,P4> mem_bind(void (C::*mem)(P1,P2,P3,P4),P obj)
	{
		details::binder4<C,P,P1,P2,P3,P4> tmp={mem,obj};
		return tmp;
	}


} } // cppcms::util

#endif
