///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SESSION_DUAL_H
#define CPPCMS_SESSION_DUAL_H

#include <cppcms/session_api.h>
#include <cppcms/defs.h>
#include <booster/hold_ptr.h>
#include <booster/shared_ptr.h>
#include <memory>

namespace cppcms {
namespace sessions {

class session_storage;
class session_sid;
class session_cookies;
class encryptor;

///
/// \brief Client and Server side storage implementation of session_api
///
class CPPCMS_API session_dual : public session_api {
public:
	///
	/// Create a new object using encryptor \a enc and session_storage \a storage.
	/// \a data_size_limit represents the maximal data size that can be stored on client side, if the data size is bigger then that
	/// the session data will be stored on server 
	///
	session_dual(	std::auto_ptr<encryptor> enc,
			booster::shared_ptr<session_storage> storage,
			size_t data_size_limit);
	///
	/// Destroy the object: release pointer to \a storage and delete an encryptor it was created with.
	///
	virtual ~session_dual();
	///
	/// See session_api::save
	///
	virtual void save(session_interface &,std::string const &data,time_t timeout,bool new_session,bool on_server);
	///
	/// See session_api::load
	///
	virtual bool load(session_interface &,std::string &data,time_t &timeout);
	///
	/// See session_api::clear
	///
	virtual void clear(session_interface &);

	///
	/// see session_api::is_blocking
	///
	virtual bool is_blocking();
private:
	struct _data;
	booster::hold_ptr<_data> d;
	booster::shared_ptr<session_cookies>	client_;
	booster::shared_ptr<session_sid>	server_;
	size_t data_size_limit_;
};

} // sessions
} // cppcms


#endif
