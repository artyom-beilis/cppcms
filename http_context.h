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
	class session_interface;
	namespace json { class value; }
	namespace impl { namespace cgi { class connection; } }

	namespace http {
		class request;
		class response;

		///
		/// \brief context is a central class that holds all specific connection related information.
		/// It encapsulates CGI request and response, cache, session and locale information
		///
		/// Instance of this class is created upon client requests, it provides access to all
		/// connection related interfaces. This class is unique per each applications hierarchy 
		/// and destroyed when HTTP request/response is completed
		///
		
		class CPPCMS_API context : public refcounted
		{
		public:
			///
			/// Internal API, don't use it
			///
			context(intrusive_ptr<impl::cgi::connection> conn);
			///
			/// Destructor.
			~context();
			///
			/// Internal API, don't use it
			///
			impl::cgi::connection &connection();

			///
			/// Get an interface to HTTP request
			///
			http::request &request();

			///
			/// Get an interface to HTTP response
			///
			http::response &response();

			///
			/// Get global settings. Same as cppcms::service::settings
			///
			json::value const &settings();

			///
			/// Get an interface to CppCMS Cache
			///
			cache_interface &cache();

			///
			/// Get an interface to current session
			///
			/// Note, when using asynchronous CppCMS applications, session data is not fetched
			/// and is not updated, because session access may be not cheap, So when using
			/// session_interface in asynchronous application make sure you call session_inerface::load
			/// member function
			///
			session_interface &session();

			///
			/// Get current context locale
			///
			std::locale locale();

			///
			/// Set locale explicitly. Note, it changes the locale of the response().out() stream as
			/// well
			///
			void locale(std::locale const &new_locale);

			///
			/// Set locale by name. Similar to locale(service().generator(name)).
			///
			/// Note: it changes the locale of the response().out() stream as well
			///
			void locale(std::string const &name);
			
			///
			/// Get the central service instance
			///
			cppcms::service &service();

			///
			/// Get current views skin name
			///
			std::string skin();

			///
			/// Set current views skin name
			///
			void skin(std::string const &name);

			///
			/// Internal API, Don't use it
			///
			void run();

			typedef enum {
				operation_completed, ///< Asynchronous operation completed successfully 
				operation_aborted    ///< Asynchronous operation was canceled
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
			static void dispatch(intrusive_ptr<application> app,std::string url,bool syncronous);
			void try_restart(bool e);
			intrusive_ptr<context> self();

			struct data;
			util::hold_ptr<data> d;
			intrusive_ptr<impl::cgi::connection> conn_;
		};

	}

};

#endif
