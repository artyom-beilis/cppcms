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
