#ifndef CPPCMS_SESSIONS_CACHE_BACKEND_H
#define CPPCMS_SESSIONS_CACHE_BACKEND_H

#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/shared_ptr.hpp>
#else // Internal Boost
#   include <cppcms_boost/shared_ptr.hpp>
    namespace boost = cppcms_boost;
#endif
#include "session_api.h"
#include "session_backend_factory.h"
#include "session_storage.h"
#include "session_sid.h"

namespace cppcms {
class worker_thread;
namespace session_cache_backend {

struct builder {
	boost::shared_ptr<session_api> operator()(worker_thread &w)
	{
		boost::shared_ptr<empty_session_server_storage> storage(new empty_session_server_storage());
		return boost::shared_ptr<session_api>(new session_sid(storage,true));
	}
};

session_backend_factory factory()
{
	return builder();
}


} // cache_backend

} //cppcms


#endif
