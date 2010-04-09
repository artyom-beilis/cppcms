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
#ifndef CPPCMS_SESSION_TCP_STORAGE_H
#define CPPCMS_SESSION_TCP_STORAGE_H

#include "session_storage.h"
#include "tcp_connector.h"
#include "session_backend_factory.h"

namespace cppcms {

class cppcms_config;

class session_tcp_storage : public session_server_storage , public tcp_connector {
protected:
	virtual unsigned hash(std::string const &key);
public:
	session_tcp_storage(std::vector<std::string> const &ips,std::vector<int> const &ports) :
		tcp_connector(ips,ports)
	{
	}
	static session_backend_factory factory(cppcms_config const &);
	virtual void save(std::string const &sid,time_t timeout,std::string const &in);
	virtual bool load(std::string const &sid,time_t *timeout,std::string &out);
	virtual void remove(std::string const &sid);
};

} // cppcms
#endif
