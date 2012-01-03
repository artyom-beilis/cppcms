///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_ERROR_CATEGORY_H
#define CPPCMS_ERROR_CATEGORY_H

#include <cppcms/config.h>
#include <booster/system_error.h>

namespace cppcms {
	namespace impl {
		namespace errc {
			enum {
				ok,
    				protocol_violation
			};
		}
		class error_category : public booster::system::error_category {
		public:
			virtual char const *name() const;
			virtual std::string message(int cat) const;
		};

		extern const error_category cppcms_category;
	}
}



#endif
