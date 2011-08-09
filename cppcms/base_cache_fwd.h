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
