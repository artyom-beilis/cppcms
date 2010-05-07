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
#ifndef CPPCMS_TCP_CACHE_SERVER_H
#define CPPCMS_TCP_CACHE_SERVER_H

#include "defs.h"
#include "base_cache.h"
#include <booster/noncopyable.h>
#include "intrusive_ptr.h"
#include <booster/hold_ptr.h>

namespace cppcms {
namespace impl {

class CPPCMS_API tcp_cache_service : public booster::noncopyable {
public:
	tcp_cache_service(intrusive_ptr<base_cache> cache,int threads,std::string ip,int port);
	~tcp_cache_service();
	void stop();
private:
	class session;
	class server;
	struct data;
	booster::hold_ptr<data> d;
};

} // impl
} // cppcms

#endif
