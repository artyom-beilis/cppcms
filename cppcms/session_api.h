///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SESSION_API_H
#define CPPCMS_SESSION_API_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/shared_ptr.h>
#include <string>

namespace cppcms {

class session_interface;


///
/// \brief This class represents the most generic implementation of session storage device
///
/// This classes are created using session_api_factory and are designed to be reimplemented by users
/// to provide an additional backends.
///
/// This class is most generic that does not provide any goodies with exception of setting or getting session cookie,
/// the user should generally take a look on sessions::session_storage or sessions::encryptor for easier implementation of session storage 
///
/// Note: these objects must be thread safe.
///
class session_api : public booster::noncopyable
{
public:
	///
	/// Save session's data:
	///
	/// \param iface - the session_interface object that allows set session cookie
	/// \param data - the data that should be stored in the session
	/// \param timeout - the absolute expiration POSIX time
	/// \param new_data - flag that marks that new session object should be created regardless of current cookie value
	/// \param on_server - flag that marks that the data must be stored on the server and not in cookies only
	///
	virtual void save(session_interface &iface,std::string const &data,time_t timeout, bool new_data, bool on_server) = 0;

	///
	/// Load session's data
	///
	/// \param iface - the session_interface object that allows read and set session cookie
	/// \param data - the string that should be filled with session data
	/// \param timeout - the expiration time of this session object
	/// \return true of session was loaded, false otherwise
	virtual bool load(session_interface &iface,std::string &data,time_t &timeout) = 0;
	///
	/// Remove the session object
	///
	/// \param iface - the session_interface object that allows read and set session cookie
	///
	virtual void clear(session_interface &iface) = 0;

	///
	/// Return true if the session store or save operations are blocking or very cpu intensive
	///
	virtual bool is_blocking() = 0;

	///
	/// Destructor
	///
	virtual ~session_api() {}
};


///
/// \brief the factory object that generates custom implemented session_api objects
///
class session_api_factory {
public:
	///
	/// Return true if this session API requires Garbage collection: i.e. execution of special function time to time
	/// to clear expired sessions from the memory
	///
	virtual bool requires_gc() = 0; 
	///
	/// The actual garbage collection job (may do nothing).
	///
	virtual void gc() = 0;

	///
	/// Return a pointer to the session_api object. Note it may be shared between multiple requests or may be created each time per request
	///
	virtual booster::shared_ptr<session_api> get() = 0;

	///
	/// Destructor and cleanup function
	///
	virtual ~session_api_factory() {}
};


} // cppcms

#endif
