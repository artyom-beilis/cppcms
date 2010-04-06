#ifndef CPPCMS_CACHE_OVER_IP_H
#define CPPCMS_CACHE_OVER_IP_H
#include "defs.h"
#include "intrusive_ptr.h"
#include "base_cache.h"
#include <string>
#include <vector>

namespace cppcms {
	namespace impl {
		intrusive_ptr<base_cache> CPPCMS_API 
		tcp_cache_factory(	std::vector<std::string> const &ips,
					std::vector<int> const &ports,
					intrusive_ptr<base_cache> l1);
				
	}
}


#endif
