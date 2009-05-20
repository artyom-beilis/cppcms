#ifndef CPPCMS_UTIL_MEM_BIND_H
#define CPPCMS_UTIL_MEM_BIND_H

namespace cppcms { namespace util {

	namespace details {
		template<typename C>
		struct binder0 {
			void (C::*member)();
			C *object;
			void operator()() { (object->*member)(); }
		};
		template<typename C,typename P1>
		struct binder1 {
			void (C::*member)(P1);
			C *object;
			void operator()(P1 p1) { (object->*member)(p1); }
		};
		template<typename C,typename P1,typename P2>
		struct binder2 {
			void (C::*member)(P1,P2);
			C *object;
			void operator()(P1 p1,P2 p2) { (object->*member)(p1,p2); }
		};
		template<typename C,typename P1,typename P2,typename P3>
		struct binder3 {
			void (C::*member)(P1,P2,P3);
			C *object;
			void operator()(P1 p1,P2 p2,P3 p3) { (object->*member)(p1,p2,p3); }
		};
		template<typename C,typename P1,typename P2,typename P3,typename P4>
		struct binder4 {
			void (C::*member)(P1,P2,P3,P4);
			C *object;
			void operator()(P1 p1,P2 p2,P3 p3,P4 p4) { (object->*member)(p1,p2,p3,p4); }
		};
	}

	template<typename C>
	details::binder0<C> mem_bind(void (C::*mem)(),C *obj)
	{
		details::binder0<C> tmp={mem,obj};
		return tmp;
	}
	template<typename C,typename P1>
	details::binder1<C,P1> mem_bind(void (C::*mem)(P1),C *obj)
	{
		details::binder1<C,P1> tmp={mem,obj};
		return tmp;
	}
	template<typename C,typename P1,typename P2>
	details::binder2<C,P1,P2> mem_bind(void (C::*mem)(P1,P2),C *obj)
	{
		details::binder2<C,P1,P2> tmp={mem,obj};
		return tmp;
	}
	template<typename C,typename P1,typename P2,typename P3>
	details::binder3<C,P1,P2,P3> mem_bind(void (C::*mem)(P1,P2,P3),C *obj)
	{
		details::binder3<C,P1,P2,P3> tmp={mem,obj};
		return tmp;
	}
	template<typename C,typename P1,typename P2,typename P3,typename P4>
	details::binder4<C,P1,P2,P3,P4> mem_bind(void (C::*mem)(P1,P2,P3,P4),C *obj)
	{
		details::binder4<C,P1,P2,P3,P4> tmp={mem,obj};
		return tmp;
	}


} } // cppcms::util

#endif
