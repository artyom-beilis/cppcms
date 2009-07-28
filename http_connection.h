#ifndef CPPCMS_HTTP_CONNECTION_H
#define CPPCMS_HTTP_CONNECTION_H

#include "defs.h"
#include "noncopyable.h"

namespace cppcms {

	namespace http {
		class request;
		class response;

		class CGICC_API connection : private util::noncopyable {
		public:
			http::request &request() = 0;
			http::response &response() = 0;
			virtual ~connection() {}
		};
	}

};

#endif
