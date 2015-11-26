///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_BINDER
#define CPPCMS_IMPL_BINDER
#include <booster/aio/types.h>
#include <booster/callback.h>
namespace cppcms {
namespace impl {


// booster::aio::handler

template<typename F,typename S>
struct handler_binder_p0 : public booster::aio::handler::callable_type {
	F f_;
	S s_;

	handler_binder_p0(F const &f,S const &s) : f_(f), s_(s) {}
	void operator()() 
	{
		((*s_).*f_)();
	}
};

template<typename C,typename S>
booster::aio::handler::pointer_type mfunc_to_handler(void (C::*f)(),S s)
{
	return new handler_binder_p0<void (C::*)(),S>(f,s);
}

template<typename F,typename S,typename P1>
struct handler_binder_p1 : public booster::aio::handler::callable_type {
	F f_;
	S s_;
	P1 p1_;

	handler_binder_p1(F const &f,S const &s, P1 const &p1) : f_(f), s_(s), p1_(p1) {}
	void operator()() 
	{
		((*s_).*f_)(p1_);
	}
};

template<typename C,typename S,typename P1,typename P1in>
booster::aio::handler::pointer_type mfunc_to_handler(void (C::*f)(P1),S s,P1in const &p1)
{
	return new handler_binder_p1<void (C::*)(P1),S,P1in>(f,s,p1);
}

template<typename F,typename S,typename P1,typename P2>
struct handler_binder_p2 : public booster::aio::handler::callable_type {
	F f_;
	S s_;
	P1 p1_;
	P2 p2_;

	handler_binder_p2(F const &f,S const &s, P1 const &p1,P2 const &p2) : f_(f), s_(s), p1_(p1),p2_(p2) {}
	void operator()() 
	{
		((*s_).*f_)(p1_,p2_);
	}
};

template<typename C,typename S,typename P1,typename P2,typename P1in,typename P2in>
booster::aio::handler::pointer_type mfunc_to_handler(void (C::*f)(P1,P2),S s,P1in const &p1,P2in const &p2)
{
	return new handler_binder_p2<void (C::*)(P1,P2),S,P1in,P2in>(f,s,p1,p2);
}




// booster::aio::event_handler

template<typename F,typename S>
struct event_handler_binder_p0 : public booster::aio::event_handler::callable_type {
	F f_;
	S s_;

	event_handler_binder_p0(F const &f,S const &s) : f_(f), s_(s) {}
	void operator()(booster::system::error_code const &e) 
	{
		((*s_).*f_)(e);
	}
};

template<typename C,typename S>
booster::aio::event_handler::pointer_type mfunc_to_event_handler(void (C::*f)(booster::system::error_code const &e),S s)
{
	return new event_handler_binder_p0<void (C::*)(booster::system::error_code const &),S>(f,s);
}

template<typename F,typename S,typename P1>
struct event_handler_binder_p1 : public booster::aio::event_handler::callable_type {
	F f_;
	S s_;
	P1 p1_;

	event_handler_binder_p1(F const &f,S const &s, P1 const &p1) : f_(f), s_(s), p1_(p1) {}
	void operator()(booster::system::error_code const &e) 
	{
		((*s_).*f_)(e,p1_);
	}
};

template<typename C,typename S,typename P1,typename P1in>
booster::aio::event_handler::pointer_type mfunc_to_event_handler(void (C::*f)(booster::system::error_code const &,P1),S s,P1in const &p1)
{
	return new event_handler_binder_p1<void (C::*)(booster::system::error_code const &,P1),S,P1in>(f,s,p1);
}

template<typename F,typename S,typename P1,typename P2>
struct event_handler_binder_p2 : public booster::aio::event_handler::callable_type {
	F f_;
	S s_;
	P1 p1_;
	P2 p2_;

	event_handler_binder_p2(F const &f,S const &s, P1 const &p1,P2 const &p2) : f_(f), s_(s), p1_(p1),p2_(p2) {}
	void operator()(booster::system::error_code const &e) 
	{
		((*s_).*f_)(e,p1_,p2_);
	}
};

template<typename C,typename S,typename P1,typename P2,typename P1in,typename P2in>
booster::aio::event_handler::pointer_type mfunc_to_event_handler(void (C::*f)(booster::system::error_code const &,P1,P2),S s,P1in const &p1,P2in const &p2)
{
	return new event_handler_binder_p2<void (C::*)(booster::system::error_code const &,P1,P2),S,P1in,P2in>(f,s,p1,p2);
}



// booster::aio::io_handler

template<typename F,typename S>
struct io_handler_binder_p0 : public booster::aio::io_handler::callable_type {
	F f_;
	S s_;

	io_handler_binder_p0(F const &f,S const &s) : f_(f), s_(s) {}
	void operator()(booster::system::error_code const &e,size_t l) 
	{
		((*s_).*f_)(e,l);
	}
};

template<typename C,typename S>
booster::aio::io_handler::pointer_type mfunc_to_io_handler(void (C::*f)(booster::system::error_code const &,size_t),S s)
{
	return new io_handler_binder_p0<void (C::*)(booster::system::error_code const &,size_t),S>(f,s);
}

template<typename F,typename S,typename P1>
struct io_handler_binder_p1 : public booster::aio::io_handler::callable_type {
	F f_;
	S s_;
	P1 p1_;

	io_handler_binder_p1(F const &f,S const &s, P1 const &p1) : f_(f), s_(s), p1_(p1) {}
	void operator()(booster::system::error_code const &e,size_t l) 
	{
		((*s_).*f_)(e,l,p1_);
	}
};

template<typename C,typename S,typename P1,typename P1in>
booster::aio::io_handler::pointer_type mfunc_to_io_handler(void (C::*f)(booster::system::error_code const &,size_t,P1),S s,P1in const &p1)
{
	return new io_handler_binder_p1<void (C::*)(booster::system::error_code const &,size_t,P1),S,P1in>(f,s,p1);
}

template<typename F,typename S,typename P1,typename P2>
struct io_handler_binder_p2 : public booster::aio::io_handler::callable_type {
	F f_;
	S s_;
	P1 p1_;
	P2 p2_;

	io_handler_binder_p2(F const &f,S const &s, P1 const &p1,P2 const &p2) : f_(f), s_(s), p1_(p1),p2_(p2) {}
	void operator()(booster::system::error_code const &e,size_t l) 
	{
		((*s_).*f_)(e,l,p1_,p2_);
	}
};

template<typename C,typename S,typename P1,typename P2,typename P1in,typename P2in>
booster::aio::io_handler::pointer_type mfunc_to_io_handler(void (C::*f)(booster::system::error_code const &,size_t,P1,P2),S s,P1in const &p1,P2in const &p2)
{
	return new io_handler_binder_p2<void (C::*)(booster::system::error_code const &,size_t,P1,P2),S,P1in,P2in>(f,s,p1,p2);
}


//// NON Member Functions

// booster::aio::handler

template<typename F,typename P1>
struct handler_fbinder_p1 : public booster::aio::handler::callable_type {
	F f_;
	
	P1 p1_;

	handler_fbinder_p1(F const &f, P1 const &p1) : f_(f), p1_(p1) {}
	void operator()() 
	{
		f_(p1_);
	}
};

template<typename C,typename P1>
booster::aio::handler::pointer_type func_to_handler(C const &f,P1 const &p1)
{
	return new handler_fbinder_p1<C,P1>(f,p1);
}

template<typename F,typename P1,typename P2>
struct handler_fbinder_p2 : public booster::aio::handler::callable_type {
	F f_;
	
	P1 p1_;
	P2 p2_;

	handler_fbinder_p2(F const &f, P1 const &p1,P2 const &p2) : f_(f), p1_(p1),p2_(p2) {}
	void operator()() 
	{
		f_(p1_,p2_);
	}
};

template<typename C,typename P1,typename P2>
booster::aio::handler::pointer_type func_to_handler(C const &f,P1 const &p1,P2 const &p2)
{
	return new handler_fbinder_p2<C,P1,P2>(f,p1,p2);
}




// booster::aio::event_handler

template<typename F,typename P1>
struct event_handler_fbinder_p1 : public booster::aio::event_handler::callable_type {
	F f_;
	
	P1 p1_;

	event_handler_fbinder_p1(F const &f, P1 const &p1) : f_(f), p1_(p1) {}
	void operator()(booster::system::error_code const &e) 
	{
		f_(e,p1_);
	}
};

template<typename C,typename P1>
booster::aio::event_handler::pointer_type func_to_event_handler(C const &f,P1 const &p1)
{
	return new event_handler_fbinder_p1<C,P1>(f,p1);
}

template<typename F,typename P1,typename P2>
struct event_handler_fbinder_p2 : public booster::aio::event_handler::callable_type {
	F f_;
	
	P1 p1_;
	P2 p2_;

	event_handler_fbinder_p2(F const &f, P1 const &p1,P2 const &p2) : f_(f), p1_(p1),p2_(p2) {}
	void operator()(booster::system::error_code const &e) 
	{
		f_(e,p1_,p2_);
	}
};

template<typename C,typename P1,typename P2>
booster::aio::event_handler::pointer_type func_to_event_handler(C const &f,P1 const &p1,P2 const &p2)
{
	return new event_handler_fbinder_p2<C,P1,P2>(f,p1,p2);
}



// booster::aio::io_handler

template<typename F,typename P1>
struct io_handler_fbinder_p1 : public booster::aio::io_handler::callable_type {
	F f_;
	
	P1 p1_;

	io_handler_fbinder_p1(F const &f, P1 const &p1) : f_(f), p1_(p1) {}
	void operator()(booster::system::error_code const &e,size_t l) 
	{
		f_(e,l,p1_);
	}
};

template<typename C,typename P1>
booster::aio::io_handler::pointer_type func_to_io_handler(C f,P1 const &p1)
{
	return new io_handler_fbinder_p1<C,P1>(f,p1);
}

template<typename F,typename P1,typename P2>
struct io_handler_fbinder_p2 : public booster::aio::io_handler::callable_type {
	F f_;
	
	P1 p1_;
	P2 p2_;

	io_handler_fbinder_p2(F const &f, P1 const &p1,P2 const &p2) : f_(f), p1_(p1),p2_(p2) {}
	void operator()(booster::system::error_code const &e,size_t l) 
	{
		f_(e,l,p1_,p2_);
	}
};

template<typename C,typename P1,typename P2>
booster::aio::io_handler::pointer_type func_to_io_handler(C f,P1 const &p1,P2 const &p2)
{
	return new io_handler_fbinder_p2<C,P1,P2>(f,p1,p2);
}










} // impl
} // cppcms


#endif
