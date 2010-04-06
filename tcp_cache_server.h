#ifndef CPPCMS_TCP_CACHE_SERVER_H
#define CPPCMS_TCP_CACHE_SERVER_H

#include "defs.h"
#include "base_cache.h"
#include "noncopyable.h"
#include "intrusive_ptr.h"
#include "hold_ptr.h"

namespace cppcms {
namespace impl {

class CPPCMS_API tcp_cache_service : public util::noncopyable {
public:
	tcp_cache_service(intrusive_ptr<base_cache> cache,int threads,std::string ip,int port);
	~tcp_cache_service();
	void stop();
private:
	class session;
	class server;
	struct data;
	util::hold_ptr<data> d;
};

} // impl
} // cppcms

#endif
