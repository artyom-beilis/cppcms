#ifndef CPPCMS_SESSION_SERVER_STORAGE_WITH_CACHE_H
#define CPPCMS_SESSION_SERVER_STORAGE_WITH_CACHE_H

#include "session_storage.h"
#include "archive.h"

namespace cppcms {

class cache_iface;

class session_server_storage_with_cache : public session_server_storage {
	cache_iface &cache;
protected:
	struct entry : public serializable {
		time_t timeout;
		std::string data;
		void load(archive &a) { a >> timeout >> data; }
		void save(archive &a) const { a <<timeout << data; }
	};
	virtual void impl_save(std::string const &sid,entry &e) = 0;
	virtual bool impl_load(std::string const &sid,entry &e) = 0;
	virtual void impl_remove(std::string const &sid) = 0;
public:
	session_server_storage_with_cache(cache_iface &cache_) :
		cache(cache_)
	{
	}
	
	virtual void save(std::string const &sid,time_t timeout,std::string const &in);
	virtual bool load(std::string const &sid,time_t *timeout,std::string &out);
	virtual void remove(std::string const &sid);
};

} // cppcms

#endif
