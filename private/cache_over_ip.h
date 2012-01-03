///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_CACHE_OVER_IP_H
#define CPPCMS_CACHE_OVER_IP_H
#include <cppcms/defs.h>
#include <booster/intrusive_ptr.h>
#include "base_cache.h"
#include <string>
#include <vector>

namespace cppcms {
	namespace impl {
		booster::intrusive_ptr<base_cache> CPPCMS_API 
		tcp_cache_factory(	std::vector<std::string> const &ips,
					std::vector<int> const &ports,
					booster::intrusive_ptr<base_cache> l1);
				
	}
}


#endif
