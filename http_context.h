#ifndef CPPCMS_HTTP_CONTEXT_H
#define CPPCMS_HTTP_CONTEXT_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"

namespace cppcms {

	class cppcms_config;
	namespace locale { class l10n; }
	namespace impl { namespace cgi { class connection; } }

	namespace http {
		class request;
		class response;

		class CPPCMS_API context : private util::noncopyable {
			struct data;
			util::hold_ptr<data> d;
			impl::cgi::connection *conn_;
		public:
			context(impl::cgi::connection *conn);
			~context();

			impl::cgi::connection &connection();
			http::request &request();
			http::response &response();
			cppcms_config const &settings();
			cppcms::locale::l10n &locale();
		};
	}

};

#endif
