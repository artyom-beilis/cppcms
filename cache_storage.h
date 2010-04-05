#ifndef THREAD_CHACHE_H
#define THREAD_CHACHE_H
#include "base_cache.h"
#include "intrusive_ptr.h"
#include "defs.h"

namespace cppcms {
namespace impl {
	intrusive_ptr<base_cache> CPPCMS_API thread_cache_factory(unsigned items);
	#ifndef CPPCMS_WIN32
	intrusive_ptr<base_cache> CPPCMS_API process_cache_factory(size_t memory);
	#endif
} // impl
} // cppcms

#endif
