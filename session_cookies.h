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
#ifndef CPPCMS_SESSION_COOKIES_H
#define CPPCMS_SESSION_COOKIES_H
#include "session_api.h"
#include <booster/hold_ptr.h>
#include "noncopyable.h"
#include <memory>
#include <string>
namespace cppcms {
class session_interface;
namespace sessions {

	class encryptor : public util::noncopyable {
	public:
		virtual std::string encrypt(std::string const &plain,time_t timeout) = 0;	
		virtual bool decrypt(std::string const &cipher,std::string &plain,time_t *timeout = 0) = 0;
		virtual ~encryptor() {}
	};

	class encryptor_factory {
	public:
		virtual std::auto_ptr<encryptor> get() = 0;
		virtual ~encryptor_factory() {}
	};

	class CPPCMS_API session_cookies : public session_api {
	public:
		session_cookies(std::auto_ptr<encryptor> encryptor);
		~session_cookies();
		virtual void save(session_interface &,std::string const &data,time_t timeout,bool newone ,bool on_server);
		virtual bool load(session_interface &,std::string &data,time_t &timeout);
		virtual void clear(session_interface &);
	private:
		struct data;
		booster::hold_ptr<data> d;
		std::auto_ptr<encryptor> encryptor_;
	};

} // sessions
} // cppcms

#endif
