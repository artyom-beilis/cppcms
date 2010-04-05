#define CPPCMS_SOURCE
#include "cache_pool.h"
#include "cache_storage.h"
#include "base_cache.h"
#include "cppcms_error.h"
#include "json.h"

namespace cppcms {

struct cache_pool::data {
	intrusive_ptr<impl::base_cache> module;
};

cache_pool::cache_pool(json::value const &settings) :
	d(new data())
{
	std::string type = settings.get("cache.backend","none");
	if(type == "none" )
		return;
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
	else {
		throw cppcms_error("Unsupported cache backend `" + type + "'");
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
