#define BOOSTER_SOURCE
#include <booster/aio/aio_category.h>

namespace booster {
namespace aio{ 
namespace aio_error {
	char const *category::name() const
	{
		return "aio::";
	}
	std::string category::message(int cat) const
	{
		switch(cat) {
		case ok: return "ok";
		case canceled: return "canceled";
		case select_failed: return "select failed";
		case eof: return "eof";
		case invalid_endpoint: return "invalid endpoint";
		case no_service_provided: return "no io_service provided";
		case prefork_not_enabled: return "prefork acceptor is not enabled";
		default:
			return "unknown";
		}
	}
} // aio_error

const aio_error::category aio_error_cat;

} // aio
} // booster
