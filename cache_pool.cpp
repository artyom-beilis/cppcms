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
	if(type=="threaded") {
		if(settings.get("service.procs",0)>1)
			throw cppcms_error("Can't use `threaded' backend with more then one process");
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
