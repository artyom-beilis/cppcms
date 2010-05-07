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

#include "session_api.h"
#include "defs.h"
#include <booster/hold_ptr.h>
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
	booster::hold_ptr<data> d;
	intrusive_ptr<session_cookies>	client_;
	intrusive_ptr<session_sid>	server_;
	size_t data_size_limit_;
};

} // sessions
} // cppcms


#endif
