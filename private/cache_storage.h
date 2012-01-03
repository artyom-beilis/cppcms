///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef THREAD_CHACHE_H
#define THREAD_CHACHE_H
#include "base_cache.h"
#include <booster/intrusive_ptr.h>
#include <cppcms/defs.h>

namespace cppcms {
namespace impl {
	booster::intrusive_ptr<base_cache> CPPCMS_API thread_cache_factory(unsigned items);
	#ifndef CPPCMS_WIN32
	booster::intrusive_ptr<base_cache> CPPCMS_API process_cache_factory(size_t memory,unsigned items);
	#endif
} // impl
} // cppcms

#endif
