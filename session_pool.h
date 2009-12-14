#ifndef CPPCMS_SESSION_POOL_H
#define CPPCMS_SESSION_POOL_H

#include "defs.h"
#include "intrusive_ptr.h"
#include "hold_ptr.h"
#include "session_api.h"

namespace cppcms {
	class service;
	
	class CPPCMS_API session_pool: public util::noncopyable {
	public:
		session_pool(service &srv);
		~session_pool();
		intrusive_ptr<session_api> get();
	private:
		struct data;
		util::hold_ptr<data> d;
	};
}


#endif
