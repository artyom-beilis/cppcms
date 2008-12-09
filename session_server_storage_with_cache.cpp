#include "session_server_storage_with_cache.h"
#include "cache_interface.h"

namespace cppcms {

void session_server_storage_with_cache::save(std::string const &sid,time_t timeout,std::string const &in)
{
	string cache_key="cppcms_sid_"+sid;
	cache.rise(cache_key);
	entry e;
	e.timeout=timeout;
	e.data=in;
	impl_save(sid,e);
	time_t now;
	time(&now);
	cache.store_data(cache_key,e,set<string>(),timeout-now);
}
bool session_server_storage_with_cache::load(std::string const &sid,time_t *timeout,std::string &out)
{
	string cache_key="cppcms_sid_"+sid;
	entry e;
	if(cache.fetch_data(cache_key,e)) {
		if(timeout) *timeout=e.timeout;
		out.swap(e.data);
		return true; 
	}
	time_t now;
	time(&now);
	if(impl_load(sid,e)) {
		if(timeout) *timeout=e.timeout;
		cache.store_data(cache_key,e,set<string>(),int(timeout-now));
		out.swap(e.data);
		return true;
	}
	return false;
}
void session_server_storage_with_cache::remove(std::string const &sid)
{
	string cache_key="cppcms_sid_"+sid;
	cache.rise(cache_key);
	impl_remove(sid);
}

} // cppcms
