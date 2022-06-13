///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/session_cookies.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/session_interface.h>
#include <cppcms/base64.h>
#include <booster/log.h>

#include <time.h>
#include <string.h>

namespace cppcms {
namespace sessions {

using namespace std;

struct session_cookies::_data {};

session_cookies::session_cookies(std::unique_ptr<encryptor> enc) :
	encryptor_(std::move(enc))
{
}
session_cookies::~session_cookies()
{
}

void session_cookies::save(session_interface &session,string const &data,time_t timeout,bool /*not_used*/,bool on_server)
{
	if(on_server)
		throw cppcms_error("Can't use cookies backend when data should be stored on server");
	std::string real_data;
	real_data.reserve(data.size() + sizeof(timeout));
	real_data.append(reinterpret_cast<char *>(&timeout),sizeof(timeout));
	real_data+=data;
	std::string cipher = encryptor_->encrypt(real_data);
	string cdata="C" + b64url::encode(cipher);
	session.set_session_cookie(cdata);
}

bool session_cookies::load(session_interface &session,string &data,time_t &timeout_out)
{
	string cdata=session.get_session_cookie();
	if(cdata.empty()) 
		return false;
	if(cdata[0]!='C') {
		session.clear_session_cookie();
		return false;
	}
	std::string cipher;
	if(!b64url::decode(cdata.substr(1),cipher)) {
		session.clear_session_cookie();
		return false;
	}
	string tmp;
	if(!encryptor_->decrypt(cipher,tmp)) {
		BOOSTER_WARNING("cppcms") << "User supplied session cookie is not valid";
		session.clear_session_cookie();
		return false;
	}
	if(tmp.size() < sizeof(time_t)) {
		session.clear_session_cookie();
		return false;
	}
	
	time_t timeout;
	memcpy(&timeout,tmp.data(),sizeof(time_t));
	if(timeout < time(0)) {
		session.clear_session_cookie();
		return false;
	}
	data = tmp.substr(sizeof(time_t));
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
