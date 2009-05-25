#ifndef CPPCMS_SESSION_API_H
#define CPPCMS_SESSION_API_H
#include "noncopyable.h"
#include <string>

namespace cppcms {

class worker_thread;
class session_interface;

class session_api : private util::noncopyable {
public:
	virtual void save(session_interface *,std::string const &data,time_t timeout, bool new_data) = 0;
	virtual bool load(session_interface *,std::string &data,time_t &timeout) = 0;
	virtual void clear(session_interface *) = 0;
	virtual ~session_api(){};
};


} // cppcms

#endif
