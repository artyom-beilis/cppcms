#ifndef CPPCMS_SESSIONS_CACHE_BACKEND_H
#define CPPCMS_SESSIONS_CACHE_BACKEND_H

#include <boost/shared_ptr.hpp>
#include "session_api.h"
#include "session_backend_factory.h"
#include "session_storage.h"
#include "session_sid.h"

namespace cppcms {
class worker_thread;
namespace session_cache_backend {

struct builder {
	shared_ptr<session_api> operator()(worker_thread &w)
	{
		boost::shared_ptr<empty_session_server_storage> storage(new empty_session_server_storage());
		return shared_ptr<session_api>(new session_sid(storage,true));
	}
};

session_backend_factory factory()
{
	return builder();
}


} // cache_backend

} //cppcms


#endif
