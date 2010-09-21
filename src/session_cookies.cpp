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
#define CPPCMS_SOURCE
#include <cppcms/session_cookies.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/session_interface.h>

#include <time.h>

namespace cppcms {
namespace sessions {

using namespace std;

struct session_cookies::_data {};

session_cookies::session_cookies(std::auto_ptr<encryptor> enc) :
	encryptor_(enc)
{
}
session_cookies::~session_cookies()
{
}

void session_cookies::save(session_interface &session,string const &data,time_t timeout,bool not_used,bool on_server)
{
	if(on_server)
		throw cppcms_error("Can't use cookies backend when data should be stored on server");
	string cdata="C" + encryptor_->encrypt(data,timeout);
	session.set_session_cookie(cdata);
}

bool session_cookies::load(session_interface &session,string &data,time_t &timeout_out)
{
	string cdata=session.get_session_cookie();
	if(cdata.empty()) return false;
	if(cdata[0]!='C') {
		session.clear_session_cookie();
		return false;
	}
	time_t timeout;
	string tmp;
	if(!encryptor_->decrypt(cdata.substr(1),tmp,&timeout))
		return false;
	if(timeout < time(0))
		return false;
	data.swap(tmp);
	timeout_out=timeout;
	return true;
}

void session_cookies::clear(session_interface &session)
{
	session.clear_session_cookie();
}

bool session_cookies::is_blocking()
{
	return false;
}


} // sessions
} // cppcms
