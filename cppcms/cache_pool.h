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
#ifndef CPPCMS_CACHE_POOL_H
#define CPPCMS_CACHE_POOL_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/intrusive_ptr.h>
#include <booster/hold_ptr.h>

namespace cppcms {
	namespace json { class value; }
	namespace impl { class base_cache; }
	
	class CPPCMS_API cache_pool {
	public:
		cache_pool(json::value const &settings);
		~cache_pool();
		booster::intrusive_ptr<impl::base_cache> get();
	private:
		struct data;
		booster::hold_ptr<data> d;
	};
}

#endif
