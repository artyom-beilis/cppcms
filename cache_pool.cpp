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
#include "cache_pool.h"
#include "cache_storage.h"
#include "base_cache.h"
#include "cppcms_error.h"
#include "cache_over_ip.h"
#include "json.h"

namespace cppcms {

struct cache_pool::data {
	intrusive_ptr<impl::base_cache> module;
};

cache_pool::cache_pool(json::value const &settings) :
	d(new data())
{
	std::string type = settings.get("cache.backend","none");
	if(type=="thread_shared") {
		if(settings.get("service.worker_processes",0)>1)
			throw cppcms_error(
				"Can't use `thread_shared' backend with more then one process ether set "
				"service.worker_processes to 0 or 1 or use cache.backend=\"process_shared\"");
		unsigned items = settings.get("cache.limit",64);
		d->module=impl::thread_cache_factory(items);
	}
	else if(type=="process_shared") {
#ifdef CPPCMS_WIN32
		throw cppcms_error("The 'process_shared' backend is not supported under MS Windows and Cygwin platforms");
#else
		size_t memory = settings.get("cache.memory",16384) * 1024;
		if(memory < 65536)
			throw cppcms_error("'process_shared' cache backend requires at least 64 KB of cache memory: cache.memory >= 64");
		d->module=impl::process_cache_factory(memory);
#endif
	}	
	else if(type != "none" ) {
		throw cppcms_error("Unsupported cache backend `" + type + "'");
	}
	
	if(settings.find("cache.tcp").type()==json::is_object) {

		std::vector<std::string> ips=settings.get<std::vector<std::string> >("cache.tcp.ips");
		std::vector<int> ports = settings.get<std::vector<int> >("cache.tcp.ports");

		if(ips.empty() || ports.empty() || ips.size()!=ports.size()) {
			throw cppcms_error("Invalid settings in cache.tcp.ports or cache.tcp.ips");
		}

		intrusive_ptr<impl::base_cache> l1=d->module;
		d->module=impl::tcp_cache_factory(ips,ports,l1);
	}
}

cache_pool::~cache_pool()
{
}

intrusive_ptr<impl::base_cache> cache_pool::get()
{
	return d->module;
}

} // cppcms
