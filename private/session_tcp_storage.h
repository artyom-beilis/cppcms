///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2011  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
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
#ifndef CPPCMS_PRIVATE_SESSION_TCP_STORAGE_H
#define CPPCMS_PRIVATE_SESSION_TCP_STORAGE_H

#include <cppcms/session_storage.h>
#include <booster/thread.h>
#include "tcp_connector.h"

namespace cppcms {
namespace sessions {

class CPPCMS_API tcp_storage : public session_storage {
public:
	tcp_storage(std::vector<std::string> const &ips,std::vector<int> const &ports) :
		ips_(ips),
		ports_(ports)
	{
	}
	virtual void save(std::string const &sid,time_t timeout,std::string const &in);
	virtual bool load(std::string const &sid,time_t &timeout,std::string &out);
	virtual void remove(std::string const &sid);
	virtual bool is_blocking();
	cppcms::impl::tcp_connector &tcp();
private:
	booster::thread_specific_ptr<cppcms::impl::tcp_connector> tcp_;
	std::vector<std::string> ips_;
	std::vector<int> ports_;
};

class CPPCMS_API tcp_factory : public session_storage_factory {
public:
	tcp_factory(std::vector<std::string> const &ips,std::vector<int> const &ports) :
		storage_(new tcp_storage(ips,ports))
	{
	}
	virtual booster::shared_ptr<session_storage> get() 
	{
		return storage_;
	}

	virtual bool requires_gc() 
	{
		return false;
	}
	virtual ~tcp_factory() {}
private:
	booster::shared_ptr<session_storage> storage_;
};

} // sessions
} // cppcms
#endif
