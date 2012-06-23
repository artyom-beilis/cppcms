///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/cache_pool.h>
#include <cppcms/config.h>
#include "cache_storage.h"
#include "base_cache.h"
#include <cppcms/cppcms_error.h>
#ifndef CPPCMS_NO_TCP_CACHE
#include "cache_over_ip.h"
#endif
#include <cppcms/json.h>

namespace cppcms {

struct cache_pool::_data {
	booster::intrusive_ptr<impl::base_cache> module;
};

cache_pool::cache_pool(json::value const &settings) :
	d(new _data())
{
	std::string type = settings.get("cache.backend","none");
#ifndef CPPCMS_NO_CACHE
	if(type=="thread_shared") {
		if(settings.get("service.worker_processes",0)>1)
			throw cppcms_error(
				"Can't use `thread_shared' backend with more then one process ether set "
				"service.worker_processes to 0 or 1 or use cache.backend=\"process_shared\"");
		unsigned items = settings.get("cache.limit",64);
		d->module=impl::thread_cache_factory(items);
	}
	else if(type=="process_shared") {
#if defined(CPPCMS_WIN32)
		throw cppcms_error("The 'process_shared' backend is not supported under MS Windows and Cygwin platforms");
#elif defined(CPPCMS_NO_PREFOK_CACHE)
		throw cppcms_error("The 'process_shared' backend is disabled during build procedure");
#else // has prefork cache
		size_t memory = settings.get("cache.memory",16384);
		if(memory < 512)
			throw cppcms_error("'process_shared' cache backend requires at least 512 KB of cache memory: cache.memory >= 512");
		unsigned items = settings.get("cache.limit",memory);
		d->module=impl::process_cache_factory(memory*1024,items);
#endif // prefork cache
	}	
	else
#endif // no cache
	if(type != "none" ) {
		throw cppcms_error("Unsupported cache backend `" + type + "'");
	}
#ifndef CPPCMS_NO_TCP_CACHE	
	if(settings.find("cache.tcp").type()==json::is_object) {

		std::vector<std::string> ips=settings.get<std::vector<std::string> >("cache.tcp.ips");
		std::vector<int> ports = settings.get<std::vector<int> >("cache.tcp.ports");

		if(ips.empty() || ports.empty() || ips.size()!=ports.size()) {
			throw cppcms_error("Invalid settings in cache.tcp.ports or cache.tcp.ips");
		}

		booster::intrusive_ptr<impl::base_cache> l1=d->module;
		d->module=impl::tcp_cache_factory(ips,ports,l1);
	}
#endif // no tcp cace
}

cache_pool::~cache_pool()
{
}

booster::intrusive_ptr<impl::base_cache> cache_pool::get()
{
	return d->module;
}

} // cppcms
