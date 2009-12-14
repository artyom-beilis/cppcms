#ifndef CPPCMS_SESSION_BACKEND_FACTORY_H
#define CPPCMS_SESSION_BACKEND_FACTORY_H

#include "defs.h"
#include "intrusive_ptr.h"

namespace cppcms {
	class session_api;
	namespace http {
		class context;
	}

	struct session_backend_factory {
	public:
		virtual intrusive_ptr<session_api> operator()(http::context &context) = 0;
		virtual ~session_backend_factory() {}
	};

} // namespace  cppcms


#endif
