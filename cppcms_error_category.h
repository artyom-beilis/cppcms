#ifndef CPPCMS_ERROR_CATEGORY_H
#define CPPCMS_ERROR_CATEGORY_H

#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/system/system_error.hpp>
#else // Internal Boost
#   include <cppcms_boost/system/system_error.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms {
	namespace impl {
		namespace errc {
			enum {
				ok,
    				protocol_violation
			};
		}
		class error_category : public boost::system::error_category {
		public:
			virtual char const *name() const;
			virtual std::string message(int cat) const;
		};

		extern const error_category cppcms_category;
	}
}



#endif
