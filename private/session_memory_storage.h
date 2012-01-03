///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SESSIOM_MEMORY_STORAGE_H
#define CPPCMS_SESSIOM_MEMORY_STORAGE_H

#include <cppcms/defs.h>
#include <cppcms/session_storage.h>
namespace cppcms {
namespace sessions {

	class CPPCMS_API session_memory_storage_factory : public session_storage_factory {
	public:
		session_memory_storage_factory();
		virtual booster::shared_ptr<session_storage> get();
		virtual bool requires_gc();
		virtual void gc_job();
		virtual ~session_memory_storage_factory();
	private:
		booster::shared_ptr<session_storage> storage_;
	};

} // sessions
} // cppcms 
#endif
