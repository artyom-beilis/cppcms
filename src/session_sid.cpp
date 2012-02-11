///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/session_sid.h>
#include <cppcms/session_storage.h>
#include <cppcms/session_interface.h>
#include <fstream>
#include <cppcms/cppcms_error.h>
#include <cppcms/urandom.h>
#include <stdio.h>
#include <time.h>

#include "tohex.h"


#include <cppcms/config.h>

namespace cppcms {
namespace sessions {


struct session_sid::_data {};

session_sid::session_sid(booster::shared_ptr<session_storage> st) :
	storage_(st)
{
}

session_sid::~session_sid()
{
}

std::string session_sid::get_new_sid()
{
	char sid[16];			
	char res[33];
	urandom_device rnd;
	rnd.generate(sid,sizeof(sid));
	cppcms::impl::tohex(sid,sizeof(sid),res);
	return res;
}

bool session_sid::valid_sid(std::string const &cookie,std::string &id)
{
	if(cookie.size()!=33 || cookie[0]!='I')
		return false;
	for(int i=1;i<33;i++) {
		char c=cookie[i];
		bool is_low_x_digit=('0'<=c && c<='9') || ('a'<=c && c<='f');
		if(!is_low_x_digit)
			return false;
	}
	id=cookie.substr(1,32);
	return true;
}

void session_sid::save(session_interface &session,std::string const &data,time_t timeout,bool new_data,bool /*unused*/)
{
	std::string id;
	if(valid_sid(session.get_session_cookie(),id)) {
		if(new_data) {
			storage_->remove(id);
			id = get_new_sid();
		}
	}
	else {
		id = get_new_sid();
	}
	storage_->save(id,timeout,data);
	session.set_session_cookie("I"+id); // Renew cookie or set new one
}

bool session_sid::load(session_interface &session,std::string &data,time_t &timeout)
{
	std::string id;
	if(!valid_sid(session.get_session_cookie(),id))
		return false;
	std::string tmp_data;
	if(!storage_->load(id,timeout,data))
		return false;
	if(time(0) > timeout) {
		storage_->remove(id);
		return false;
	}
	return true;
}

void session_sid::clear(session_interface &session)
{
	std::string id;
	if(valid_sid(session.get_session_cookie(),id))
		storage_->remove(id);
	session.clear_session_cookie();
}

bool session_sid::is_blocking()
{
	return storage_->is_blocking();
}


} // sessions
} // namespace cppcms
