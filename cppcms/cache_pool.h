///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_CACHE_POOL_H
#define CPPCMS_CACHE_POOL_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/intrusive_ptr.h>
#include <booster/hold_ptr.h>
#include <cppcms/base_cache_fwd.h>

namespace cppcms {
	namespace json { class value; }
	namespace impl { class base_cache; }

	/// \cond INTERNAL
	class CPPCMS_API cache_pool {
	public:
		cache_pool(json::value const &settings);
		~cache_pool();
		booster::intrusive_ptr<impl::base_cache> get();
	private:
		struct _data;
		booster::hold_ptr<_data> d;
	};
	/// \endcond
}

#endif
