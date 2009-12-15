#ifndef CPPCMS_SESSION_DUAL_H
#define CPPCMS_SESSION_DUAL_H

#include "session_api.h"
#include "defs.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"
#include <memory>

namespace cppcms {
namespace sessions {

class session_storage;
class session_sid;
class session_cookies;
class encryptor;

class CPPCMS_API session_dual : public session_api {
public:
	session_dual(	std::auto_ptr<encryptor> enc,
			intrusive_ptr<session_storage> storage,
			size_t data_size_limit);
	virtual ~session_dual();
	virtual void save(session_interface &,std::string const &data,time_t timeout,bool new_session,bool on_server);
	virtual bool load(session_interface &,std::string &data,time_t &timeout);
	virtual void clear(session_interface &);
private:
	struct data;
	util::hold_ptr<data> d;
	intrusive_ptr<session_cookies>	client_;
	intrusive_ptr<session_sid>	server_;
	size_t data_size_limit_;
};

} // sessions
} // cppcms


#endif
