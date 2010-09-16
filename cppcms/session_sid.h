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

#include <cppcms/session_api.h>
#include <cppcms/defs.h>
#include <booster/hold_ptr.h>
#include <booster/shared_ptr.h>
#include <cppcms/session_storage.h>

namespace cppcms {
namespace sessions {

	namespace impl { class sid_generator; }
	
	///
	/// \brief An implementation of session_api that stores the data using session_storage and unique session id.
	///
	class CPPCMS_API session_sid : public session_api {
	public:
		///
		/// Create a new session_sid with a pointer \a s to session_storage
		///
		session_sid(booster::shared_ptr<session_storage> s);
		///
		/// Delete an object and release a session_storage it used.
		///
		~session_sid();
		///
		/// See session_api::save
		///
		virtual void save(session_interface &,std::string const &data,time_t timeout,bool,bool);
		///
		/// See session_api::load
		///
		virtual bool load(session_interface &,std::string &data,time_t &timeout);
		///
		/// See session_api::is_blocking
		///
		virtual bool is_blocking();
		///
		/// See session_api::clear
		///
		virtual void clear(session_interface &);

	private:

		std::string get_new_sid();
		bool valid_sid(std::string const &cookie,std::string &id);
		
		struct _data;
		booster::hold_ptr<_data> d;
		booster::shared_ptr<session_storage> storage_;
	};

} // sessions
} // cppcms


#endif
