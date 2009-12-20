#ifndef CPPCMS_SESSION_API_H
#define CPPCMS_SESSION_API_H

#include "defs.h"
#include "noncopyable.h"
#include "refcounted.h"
#include "intrusive_ptr.h"
#include <string>

namespace cppcms {

class session_interface;

class session_api : 
	public util::noncopyable,
	public refcounted
{
public:
	virtual void save(session_interface &,std::string const &data,time_t timeout, bool new_data, bool on_server) = 0;
	virtual bool load(session_interface &,std::string &data,time_t &timeout) = 0;
	virtual void clear(session_interface &) = 0;
	virtual ~session_api() {}
};

class session_api_factory {
public:
	virtual bool requires_gc() = 0; 
	virtual void gc() = 0;
	virtual intrusive_ptr<session_api> get() = 0;
	virtual ~session_api_factory() {}
};


} // cppcms

#endif
