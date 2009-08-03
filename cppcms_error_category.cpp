#include "cppcms_error_category.h"

namespace cppcms {
namespace impl {
	char const *error_category::name() const
	{
		return "cppcms::io";
	}
	std::string error_category::message(int cat) const
	{
		switch(cat) {
		case errc::ok: return "ok";
		case errc::protocol_violation: return "protocol violation";
		default:
			return "unknown";
		}
	}
	const error_category cppcms_category;

} // impl
} // cppcms
