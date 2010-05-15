//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_ERROR_CATEGORY_H
#define BOOSTER_ERROR_CATEGORY_H


#include <booster/system_error.h>

namespace booster {
namespace aio {

	namespace aio_error {
		enum {
			ok,
			canceled,
			select_failed,
			eof,
			invalid_endpoint,
			no_service_provided,
			prefork_not_enabled
		};

		class BOOSTER_API category : public system::error_category {
		public:
			virtual char const *name() const;
			virtual std::string message(int cat) const;
		};
		
		BOOSTER_API aio_error::category const &get_category();
	}

	static aio_error::category const &aio_error_cat = aio_error::get_category();
}
} // booster


#endif
