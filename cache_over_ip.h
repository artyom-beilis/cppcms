///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
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
