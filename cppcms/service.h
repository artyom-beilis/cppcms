///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SERVICE_H
#define CPPCMS_SERVICE_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <booster/function.h>
#include <locale>
#include <memory>
#include <cppcms/locale_fwd.h>
#include <cppcms/json.h>

namespace booster {
	namespace aio {
		class io_service;
	}
}

///
/// \brief This is the namespace where all CppCMS functionality is placed
///
namespace cppcms {
	namespace impl {
		struct cached_settings;
		class service;
		namespace cgi {
			class acceptor;
		}
	}

	class applications_pool;
	class thread_pool;
	class session_pool;
	class cache_pool;
	class views_pool;
	class forwarder; 
	
	namespace json {
		class value;
	}

	///
	/// \brief This class represent the central event loop of the CppCMS applications.
	///
	/// This is the central class that is responsible for management of CppCMS services: management of HTTP requests and
	/// responses, sessions, cache and much more services. It is the central class that runs during execution of any CppCMS
	/// service.
	///

	class CPPCMS_API service : public booster::noncopyable
	{
	public:
		///
		/// Load configuration settings from the command line parameters. This object can be passed
		/// to service(json::value const &v) constructor allowing to alter these settings before
		/// creating the actual service object.
		///
		static json::value load_settings(int argc,char *argv[]);
		///
		/// Create a new service, passing all configuration settings via json::value instead of parsing command line parameters.
		///
		service(json::value const &v);
		///
		/// Parse command line parameters, get the configuration file and additional options and create the service.
		///
		/// Note for Windows users: argv is assumed to be UTF-8 encoded strings, if you want pass non-ascii or "wide" parameters
		/// you need convert them from UTF-16 to UTF-8. For example you can use booster::nowide::convert functions.
		///
		service(int argc,char *argv[]);
		///
		/// Destroy the service - the last class to go down.
		///
		/// Note, if you have asynchronous applications that are not owned the service class they must be destroyed before it
		///
		~service();

		///
		/// Start the central even loop of CppCMS service. By default, it also install signal handlers on POSIX platforms
		/// to handle SIGINT, SIGTERM and SIGUSR1 signals or uses SetConsoleCtrlHandler on Windows platforms waiting on
		/// C, BREAK, CLOSE and SHUTDOWN events.
		///
		/// This even call service::shutdown function that stops the main event loop and shutdowns the service properly.
		///
		/// If you want to disable these handlers, you may set service.disable_global_exit_handling option to true.
		///
		void run();

		///
		/// Stop the service event loop. This member function is both thread and signal safe.
		///
		void shutdown();

		///
		/// Get the configuration setting the service was created with.
		///
		json::value const &settings();
		
		///
		/// Get a cppcms::applications_pool instance of this service
		///
		cppcms::applications_pool &applications_pool();
		///
		/// Get a cppcms::thread_pool instance of this service
		///
		cppcms::thread_pool &thread_pool();
		///
		/// Get a cppcms::session_pool instance of this service, never used directly by user level applications 
		///
		cppcms::session_pool &session_pool();
		///
		/// Get the cppcms::views_pool instance, note the skins management is still performed in the singleton instance of cppcms::views_pool
		///
		cppcms::views_pool &views_pool();
		///
		/// Get the cppcms::cache_pool instance of this service, never used directly by user level applications
		///
		cppcms::cache_pool &cache_pool();
		///
		/// Get cppcms::forwarder instance of this service
		///
		cppcms::forwarder &forwarder();

		///
		/// Get locale::generator instance of this service
		///
		locale::generator const &generator();
		///
		/// Get the default locale for this service.
		///
		std::locale locale();
		///
		/// Get a locale by \a name, shortcut to generator().get(name);
		///
		std::locale locale(std::string const &name);


		///
		/// Get low level central event loop of the CppCMS service that allows you to connect asynchronous application with generic
		/// asynchronous sockets API or create any asynchronous TCP/IP servers over it.
		///
		booster::aio::io_service &get_io_service();

		///
		/// Post an execution handler \a handler to the event loop queue. This member function is thread safe allowing 
		/// safe communication between event loop thread (where all asynchronous applications run) and any other threads.
		///
		void post(booster::function<void()> const &handler);

		///
		/// Execute handler after forking processes (on POSIX platforms). This is useful when you want to start various asynchronous
		/// applications or services in separate processes.
		///
		void after_fork(booster::function<void()> const &handler);
		
		///
		/// Get the size of thread pool each of CppCMS processes will be running.
		///
		int threads_no();

		///
		/// Get the number of forked processes used by the service. Always 0 on Windows and Cygwin.
		///
		int procs_no();

		///
		/// Get current process identification number. Note, this is not getpid() function.
		///
		/// When CppCMS service manages preforked process pool, each process gets its own id starting from 1 to procs_no() when
		/// the parent supervisor process gets an id 0. If one of the forked processes crash, it is replaced with a new process
		/// with same id.
		///
		/// So if an application want to run certain activity that can't be executed in multiple processes it can setup an after_fork
		/// handle that would check the process id and start this activity.
		///
		/// Under Windows and Cygwin it is always 0.
		///
		int process_id();

		/// \cond INTERNAL
		
		// internal functions never call it directly

		cppcms::impl::service &impl();

		impl::cached_settings const &cached_settings();
		
		#ifdef CPPCMS_WIN_NATIVE
		void impl_run_as_windows_service();
		#endif
		void impl_run_prepare();
		void impl_run_event_loop();
		
		/// \endcond

	private:
		void setup();
		void setup_logging();
		std::auto_ptr<cppcms::impl::cgi::acceptor> setup_acceptor(json::value const &,int,int shift=0);
		void stop();
		void start_acceptor(bool after_fork=false);
		void setup_exit_handling();
		bool prefork();
		void impl_run();
		#ifdef CPPCMS_WIN_NATIVE
		void install_service();
		void uninstall_service();
		#endif
		booster::hold_ptr<impl::service> impl_;
	};

} //




#endif
