#ifndef CPPCMS_HTTP_CONTEXT_H
#define CPPCMS_HTTP_CONTEXT_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"

namespace cppcms {

	class cppcms_config;
	namespace cgi { class io; }

	namespace http {
		class request;
		class response;

		class CPPCMS_API context : private util::noncopyable {
			util::hold_ptr<cgi::io> io_;
		public:
			http::request &request();
			http::response &response();
			cppcms_config const &settings();
			cgi::io &io();
		};
	}

};

#endif
