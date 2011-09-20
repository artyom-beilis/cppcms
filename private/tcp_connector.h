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
#ifndef CPPCMS_TCP_CONNECTOR_H
#define CPPCMS_TCP_CONNECTOR_H

#include <string>
#include <vector>
#include <booster/noncopyable.h>

namespace cppcms {

namespace impl {
class messenger;
struct tcp_operation_header;

class tcp_connector : private booster::noncopyable
{
public:
	messenger &get(std::string const &key);

	tcp_connector(std::vector<std::string> const &ip_list,std::vector<int> const &port_list);
	virtual ~tcp_connector();
	
	void broadcast(tcp_operation_header &h,std::string &data);

protected:
	messenger *tcp;
	int conns;
	virtual unsigned hash(std::string const &key);
};

} // impl
} // cppcms


#endif
