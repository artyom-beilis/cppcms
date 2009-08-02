#ifndef CPPCMS_ERROR_CATEGORY_H
#define CPPCMS_ERROR_CATEGORY_H

#include <boost/system/system_error.hpp>

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
			virtual char const *name() const
			{
				return "cppcms::io";
			}
			virtual std::string message(int cat) const
			{
				switch(cat) {
				case errc::ok: return "ok";
				case errc::protocol_violation: return "protocol violation";
				default:
					return "unknown";
				}
			}
		};

		extern const error_category cppcms_category;
	}
}



#endif
