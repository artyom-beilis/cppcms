#ifndef CPPCMS_SESSION_BACKEND_FACTORY_H
#define CPPCMS_SESSION_BACKEND_FACTORY_H

#include <string>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/function.hpp>
#   include <boost/shared_ptr.hpp>
#else // Internal Boost
#   include <cppcms_boost/function.hpp>
#   include <cppcms_boost/shared_ptr.hpp>
    namespace boost = cppcms_boost;
#endif
namespace cppcms {

class session_api;
class worker_thread;

typedef boost::shared_ptr<session_api> session_backend_shared_ptr;
typedef boost::function<session_backend_shared_ptr(worker_thread &)> session_backend_factory;

} // namespace  cppcms


#endif
