///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_BASE_CACHE_FWD_H
#define CPPCMS_BASE_CACHE_FWD_H

#include <string>
#include <set>
#include <cppcms/defs.h>
#include <booster/intrusive_ptr.h>
#include <cppcms/cstdint.h>

namespace cppcms {
	namespace impl {
		class base_cache;
		inline void intrusive_ptr_add_ref(base_cache *ptr);
		inline void intrusive_ptr_release(base_cache *ptr);
	} // impl
} //cppcms

#endif
