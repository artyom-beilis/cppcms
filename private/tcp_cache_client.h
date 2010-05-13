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
#ifndef TCP_CHACHE_H
#define TCP_CHACHE_H
#include "base_cache.h"
#include "cache_interface.h"
#include "session_storage.h"
#include "tcp_connector.h"
#include <string>

namespace cppcms {
namespace impl {
	class messenger;
	struct tcp_operation_header;


	class tcp_cache : public tcp_connector {
	public:

		tcp_cache(	std::vector<std::string> const &ip_list,
				std::vector<int> const &port_list) 
		:
			tcp_connector(ip_list,port_list)
		{
		}

		static const int up_to_date = -1;
		static const int not_found = 0;
		static const int found = 1;

		int fetch(	std::string const &key,
				std::string &data,
				std::set<std::string> *triggers,
				time_t &timeout,
				uint64_t &generation,
				bool transfer_if_not_updated=false);
		void rise(std::string const &trigger);
		void clear();
		void stats(unsigned &keys,unsigned &triggers);
		void store(	std::string const &key,
				std::string const &data,
				std::set<std::string> const &triggers,
				time_t timeout);
		~tcp_cache();
	};

} // impl
} // cppcms

#endif
