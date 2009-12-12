#ifndef THREAD_CHACHE_H
#define THREAD_CHACHE_H
#include "base_cache.h"
#include "intrusive_ptr.h"

#include <map>
#include <list>

using namespace std;

namespace cppcms {
namespace impl {
	intrusive_ptr<base_cache> thread_cache_factory(unsigned items);
} // impl
} // cppcms

#endif
