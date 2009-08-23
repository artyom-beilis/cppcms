#ifndef CPPCMS_HTTP_CONTEXT_H
#define CPPCMS_HTTP_CONTEXT_H

#include "defs.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"
#include "refcounted.h"

namespace cppcms {

	class cppcms_config;
	class service;

	namespace locale { class environment; }
	namespace impl { namespace cgi { class connection; } }

	namespace http {
		class request;
		class response;

		class CPPCMS_API context : public refcounted
		{
		public:
			context(intrusive_ptr<impl::cgi::connection> conn);
			~context();

			impl::cgi::connection &connection();
			http::request &request();
			http::response &response();
			cppcms_config const &settings();
			cppcms::locale::environment &locale();
			cppcms::service &service();

			void run();
			void on_response_complete();
		private:
			void on_request_ready(bool error);
			static void dispatch(intrusive_ptr<application> app,bool syncronous);
			void try_restart(bool e);
			intrusive_ptr<context> self();

			struct data;
			util::hold_ptr<data> d;
			intrusive_ptr<impl::cgi::connection> conn_;
		};

	}

};

#endif
