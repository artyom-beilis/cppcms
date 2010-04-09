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
#ifndef CPPCMS_SESSION_SID_H
#define CPPCMS_SESSION_SID_H

#include "session_api.h"
#include "defs.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"
#include "session_storage.h"

namespace cppcms {
namespace sessions {

	namespace impl { class sid_generator; }
	
	class CPPCMS_API session_sid : public session_api {
	public:
		session_sid(intrusive_ptr<session_storage> s);
		~session_sid();
		virtual void save(session_interface &,std::string const &data,time_t timeout,bool,bool);
		virtual bool load(session_interface &,std::string &data,time_t &timeout);
		virtual void clear(session_interface &);
	private:

		bool valid_sid(std::string const &cookie,std::string &id);
		std::string key(std::string sid);
		
		struct data;
		util::hold_ptr<data> d;
		intrusive_ptr<session_storage> storage_;
	};

} // sessions
} // cppcms


#endif
