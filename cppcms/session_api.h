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
#ifndef CPPCMS_SESSION_API_H
#define CPPCMS_SESSION_API_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/shared_ptr.h>
#include <string>

namespace cppcms {

class session_interface;

class session_api : public booster::noncopyable
{
public:
	virtual void save(session_interface &,std::string const &data,time_t timeout, bool new_data, bool on_server) = 0;
	virtual bool load(session_interface &,std::string &data,time_t &timeout) = 0;
	virtual void clear(session_interface &) = 0;
	virtual ~session_api() {}
};

class session_api_factory {
public:
	virtual bool requires_gc() = 0; 
	virtual void gc() = 0;
	virtual booster::shared_ptr<session_api> get() = 0;
	virtual ~session_api_factory() {}
};


} // cppcms

#endif
