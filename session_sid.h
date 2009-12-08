#ifndef CPPCMS_SESSION_SID_H
#define CPPCMS_SESSION_SID_H

#include <sys/time.h>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/shared_ptr.hpp>
#else // Internal Boost
#   include <cppcms_boost/shared_ptr.hpp>
    namespace boost = cppcms_boost;
#endif
#include "session_api.h"

namespace cppcms {
	
class session_server_storage;
class session_interface;
	
namespace details {
class sid_generator : public util::noncopyable {
	struct for_hash {
		char uid[16];
		uint64_t session_counter;
		struct timeval tv;
	} hashed;
public:
	sid_generator();
	std::string operator()();
};
}

class session_sid : public session_api {
	details::sid_generator sid;
	boost::shared_ptr<session_server_storage> storage;
	bool cache;
	std::string key(std::string sid);
public:
	bool valid_sid(std::string const &str);
	session_sid(boost::shared_ptr<session_server_storage> s,bool c=false) :
		storage(s),
		cache(c)
	{
	}
	virtual void save(session_interface *,std::string const &data,time_t timeout,bool);
	virtual bool load(session_interface *,std::string &data,time_t &timeout);
	virtual void clear(session_interface *);

};
}


#endif
