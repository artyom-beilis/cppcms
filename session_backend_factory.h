#ifndef CPPCMS_SESSION_BACKEND_FACTORY_H
#define CPPCMS_SESSION_BACKEND_FACTORY_H

#include <string>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
namespace cppcms {

class session_api;
class worker_thread;

typedef boost::shared_ptr<session_api> session_backend_shared_ptr;
typedef boost::function<session_backend_shared_ptr(worker_thread &)> session_backend_factory;

} // namespace  cppcms


#endif
