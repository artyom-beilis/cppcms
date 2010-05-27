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
#include <cppcms/session_dual.h>
#include <cppcms/session_interface.h>
#include <cppcms/session_cookies.h>
#include <cppcms/session_sid.h>

using namespace std;

namespace cppcms {
namespace sessions {

struct session_dual::_data {};

session_dual::session_dual(std::auto_ptr<encryptor> enc,booster::shared_ptr<session_storage> storage,size_t limit) :
	client_(new session_cookies(enc)),
	server_(new session_sid(storage)),
	data_size_limit_(limit)
{
}

session_dual::~session_dual()
{
}

void session_dual::save(session_interface &session,string const &data,time_t timeout,bool isnew,bool on_server)
{
	if(on_server || data.size() > data_size_limit_) {
		server_->save(session,data,timeout,isnew,true);
	}
	else {
		std::string cookie=session.get_session_cookie();
		if(!cookie.empty() && cookie[0]=='I') {
			server_->clear(session);
		}
		client_->save(session,data,timeout,isnew,false);
	}
}

bool session_dual::load(session_interface &session,string &data,time_t &timeout)
{
	std::string cookie = session.get_session_cookie();
	if(!cookie.empty() && cookie[0]=='C') {
		return client_->load(session,data,timeout);
	}
	else {
		return server_->load(session,data,timeout);
	}
}

void session_dual::clear(session_interface &session)
{
	std::string cookie = session.get_session_cookie();
	if(!cookie.empty() && cookie[0]=='C') {
		client_->clear(session);
	}
	else {
		server_->clear(session);
	}
}

} // sessions
} // cppcms
