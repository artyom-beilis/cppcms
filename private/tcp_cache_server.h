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

#include <cppcms/defs.h>
#include "base_cache.h"
#include <booster/noncopyable.h>
#include <booster/intrusive_ptr.h>
#include <booster/shared_ptr.h>
#include <booster/hold_ptr.h>

namespace cppcms {
namespace sessions {
	class session_storage_factory;
}
namespace impl {

class CPPCMS_API tcp_cache_service : public booster::noncopyable {
public:
	tcp_cache_service(	booster::intrusive_ptr<base_cache> cache,
				booster::shared_ptr<cppcms::sessions::session_storage_factory> f,
				int threads,
				std::string ip,
				int port,
				int gc_timeout = 10);
	~tcp_cache_service();
	void stop();
private:
	class session;
	class server;
	struct _data;
	booster::hold_ptr<_data> d;
};

} // impl
} // cppcms

#endif
