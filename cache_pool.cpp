#define CPPCMS_SOURCE
#include "cache_pool.h"
#include "thread_cache.h"
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
