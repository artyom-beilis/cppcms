#ifndef CPPCMS_HTTP_CONTEXT_H
#define CPPCMS_HTTP_CONTEXT_H

#include "defs.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"
#include "refcounted.h"
#include "callback0.h"
#include "callback1.h"

#include <locale>

namespace cppcms {

	class service;
	class application;
	class cache_interface;
	namespace json { class value; }
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
			json::value const &settings();
			cache_interface &cache();
			std::locale locale();
			void locale(std::locale const &new_locale);
			void locale(std::string const &name);
			cppcms::service &service();

			std::string skin();
			void skin(std::string const &name);

			void run();

			typedef enum {
				operation_completed,
				operation_aborted
			} complition_type;

			typedef util::callback1<complition_type> handler;

			///
			/// Send all pending output data to the client and
			/// finalize the connection. Note, you can't use this 
			/// object for communication any more.
			///
			void async_complete_response();
			
			///
			/// Send all pending data to user, when operation is complete
			/// call handler \a h with status.
			///
			/// Note: if the status is operation_aborted, you can't use
			/// this connection any more, the peer gone.
			///

			void async_flush_output(handler const &h);

			///
			/// Set handler for peer reset events. It is useful to cleanup
			/// connections that had timeout or just disconnected by user
			///
			/// Notes: 
			/// 1. if async_complete_response was called, handler would not
			///    be called any more.
			/// 2. If async_flush_output fails, this does not mean that 
			///    this handler would be called as well, so you need to check both
			///
			void async_on_peer_reset(util::callback0 const &h);
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
