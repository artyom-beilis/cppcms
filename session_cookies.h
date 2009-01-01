#ifndef CPPCMS_SESSION_COOKIES_H
#define CPPCMS_SESSION_COOKIES_H
#include "session_api.h"
#include <memory>
#include <string>
#include "session_backend_factory.h"

namespace cppcms {

class encryptor;
class worker_thread;
class session_interface;

class session_cookies : public session_api {
	worker_thread &worker;
	std::auto_ptr<encryptor> encr;
public:
	static session_backend_factory factory();
	session_cookies(worker_thread &w);
	session_cookies(worker_thread &w,std::auto_ptr<encryptor>);
	virtual void save(session_interface *,std::string const &data,time_t timeout,bool );
	virtual bool load(session_interface *,std::string &data,time_t &timeout);
	virtual void clear(session_interface *);
};


} // cppcms

#endif
