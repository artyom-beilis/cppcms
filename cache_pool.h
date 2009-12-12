#ifndef CPPCMS_CACHE_POOL_H
#define CPPCMS_CACHE_POOL_H

#include "defs.h"
#include "noncopyable.h"
#include "intrusive_ptr.h"
#include "hold_ptr.h"

namespace cppcms {
	namespace json { class value; }
	namespace impl { class base_cache; }
	
	class CPPCMS_API cache_pool {
	public:
		cache_pool(json::value const &settings);
		~cache_pool();
		intrusive_ptr<impl::base_cache> get();
	private:
		struct data;
		util::hold_ptr<data> d;
	};
}

#endif
