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
#if defined(__sun)
#define _POSIX_PTHREAD_SEMANTICS
#endif
#include "tcp_cache_server.h"
#include "cache_storage.h"
#ifndef CPPCMS_WIN32
#include <signal.h>
#include "daemonize.h"
#endif
#ifdef CPPCMS_WIN_NATIVE
#include "winservice.h"
#endif
#include <iostream>
#include <stdlib.h>
#include <stdexcept>
#include <booster/shared_ptr.h>
#include <booster/intrusive_ptr.h>
#include <booster/shared_object.h>
#include <booster/thread.h>
#include <booster/log.h>
#include "base_cache.h"
#include "logging.h"
#include <cppcms/session_storage.h>
#include <cppcms/json.h>
#include <cppcms/service.h>
#include <cppcms/cppcms_error.h>

#ifdef CPPCMS_WIN_NATIVE
#include "session_win32_file_storage.h"
#include "winservice.h"
#else
#include "session_posix_file_storage.h"
#endif
#include "session_memory_storage.h"


struct settings {
	std::string ip;
	int port;
	int threads;
	int gc;
	cppcms::json::value config;
	
	booster::shared_ptr<cppcms::sessions::session_storage_factory> sessions;
	booster::shared_object plugin;
	booster::intrusive_ptr<cppcms::impl::base_cache> cache;
	
	~settings()
	{
		sessions.reset(); // ensure that sessions object is destroyed before shared object
		cache = 0;
	}
	
	settings(int argc,char **argv)
	{
		try {
			config = cppcms::service::load_settings(argc,argv);
			setup(config);
		}
		catch(cppcms::cppcms_error const &) {
			help();
			throw;
		}
	}
	void help()
	{
		std::cerr <<
		"usage: cppcms_scale [ -c config.js ] [ parameters ]\n"
		"  -c config.js       JSON, Configuration file, also parameters may\n"
		"                     be set via command line rather then the file\n"
		" --ip=IP             IP to bind, default 0.0.0.0\n"
		" --port=N            Port to bind, mandatory\n"
		" --threads=N         Worker threads, default = # HW CPU\n"
		" --cache-limit=N     The size of the cache in items\n"
		" --session-storage=(memory|files|external)\n"
		"                     Session storage module\n"
		" --session-gc=N      The frequency of garbage collection\n"
		" --session-dir=/path The location of files for session storage\n"
		" --session-shared_object=/path\n"
		"                     The shared object/dll that is used as external\n"
		"                     storage"
		" --session-module=name\n"
		"                     The name of the module for example mymod for\n"
		"                     an external storage, renamed to shared object\n"
		"                     according to OS conventions, like libmymod.so\n"
#ifndef CPPCMS_WIN32
		" --daemon-enable=(true|false)\n"
		"                     Run the process as daemon, default false\n"
		" --daemon-lock=/path Location of the lock file that keeps process pid\n"
		" --daemon-user=user  User that the daemon should run under\n"
		" --daemon-group=grp  Group that the daemon should run under\n"
		" --daemon-chroot=/path\n"
		"                     Run the service in chroot jain\n"
		" --daemon-fdlimit=N  The limit of file descriptors for the service\n"
#endif
#ifdef CPPCMS_WIN_NATIVE
		" --winservice-mode=(install|uninstall)\n"
		"                     Install or uninstall windows service\n"
		" --winservice-name=XXX\n"
		"                     Name of windows service\n"
		" --winservice-display_name=XXX\n"
		"                     The service name shown in the UI\n"
		" --winservice-start=(auto|demand)\n"
		"                     Windows service start mode\n"
		" --winservice-user=XXX\n"
		" --winservice-password=XXX\n"
		"                     The user and password the service would run under\n"
#endif
		<< std::endl;
	}
	void setup(cppcms::json::value const &v)
	{		
		ip=v.get("ip","0.0.0.0");
		port=v.get<int>("port");
		threads=v.get("threads",booster::thread::hardware_concurrency());
		int items = v.get("cache.limit",-1);
		if(items!=-1){
			cache = cppcms::impl::thread_cache_factory(items);
		}
		gc=v.get("session.gc",10);
		std::string stor = v.get("session.storage","");
		if(!stor.empty()) {
			if(stor == "files") {
				std::string dir = v.get("session.dir","");
				#ifdef CPPCMS_WIN_NATIVE
				sessions.reset(new cppcms::sessions::session_file_storage_factory(dir));
				#else
				sessions.reset(new cppcms::sessions::session_file_storage_factory(dir,threads,1,false));
				#endif
			}
			else if(stor == "memory") {
				sessions.reset(new cppcms::sessions::session_memory_storage_factory());
			}
			else if(stor == "external") {
				std::string so = v.get<std::string>("session.shared_object","");
				std::string module = v.get<std::string>("session.module","");
				std::string entry_point = v.get<std::string>("session.entry_point","sessions_generator");
				if(so.empty() && module.empty())
					throw cppcms::cppcms_error(
								"session.storage=external "
								"and neither session.shared_object "
								"nor session.module is defined");
				if(!so.empty() && !module.empty())
					throw cppcms::cppcms_error(
								"both session.shared_object "
								"and session.module are defined");

				if(so.empty()) {
					so = booster::shared_object::name(module);
				}
				std::string error;
				if(!plugin.open(so,error)) {
					throw cppcms::cppcms_error("sessions_pool: failed to load shared object " + so + ": " + error);
				}
				cppcms::sessions::cppcms_session_storage_generator_type f=0;
				plugin.symbol(f,entry_point);
				sessions.reset(f(v.find("session.settings")));
			}
			else 
				throw cppcms::cppcms_error("Unknown session.storage:"+stor);
		}
		if(!sessions && !cache) {
			throw cppcms::cppcms_error("Neither cache.limit nor session.storage is defined");
		}
	}

};

#if !defined(CPPCMS_WIN32)
void main_posix(settings &par)
{
	cppcms::impl::daemonizer demon(par.config);

	cppcms::impl::tcp_cache_service srv(
		par.cache,
		par.sessions,
		par.threads,
		par.ip,
		par.port,
		par.gc);
	
	// Wait for signals for exit
	sigset_t wait_mask;
	sigemptyset(&wait_mask);
	sigaddset(&wait_mask, SIGINT);
	sigaddset(&wait_mask, SIGQUIT);
	sigaddset(&wait_mask, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
	int sig = 0;
	sigwait(&wait_mask, &sig);
	
	BOOSTER_NOTICE("cppcms_scale")<<"Catch signal: exiting...";

	srv.stop();
}
#elif defined(CPPCMS_WIN_NATIVE)

static booster::shared_ptr<cppcms::impl::tcp_cache_service> the_server;
static settings *the_settings;
static booster::mutex done_lock;
static booster::condition_variable done_cond;
static bool done;

static void win_prepare()
{
	BOOSTER_NOTICE("cppcms_scale")<<"Starting service...";
	settings &par = *the_settings;
	the_server.reset(
		new cppcms::impl::tcp_cache_service(
			par.cache,
			par.sessions,
			par.threads,
			par.ip,
			par.port,
			par.gc)
	);
	
}

static void win_stop()
{
	booster::unique_lock<booster::mutex> guard(done_lock);
	done = true;
	done_cond.notify_all();
}

static void win_run()
{
	{
		booster::unique_lock<booster::mutex> guard(done_lock);
		while(!done) {
			done_cond.wait(guard);
		}
	}
	BOOSTER_NOTICE("cppcms_scale")<<"Stopping service...";
	the_server->stop();
	the_server.reset();
	the_settings = 0;
}

void main_win(settings &par,int argc,char **argv)
{
	using cppcms::impl::winservice;
	
	the_settings = &par;
	winservice::instance().prepare(win_prepare);
	winservice::instance().stop(win_stop);
	winservice::instance().exec(win_run);
	
	winservice::instance().run(par.config,argc,argv);
	
}

#endif

#ifdef CPPCMS_WIN32

void main_console(settings &par)
{
	cppcms::impl::tcp_cache_service srv(
		par.cache,
		par.sessions,
		par.threads,
		par.ip,
		par.port,
		par.gc);

	std::cout << "Press any key to stop..." << std::flush;
	std::cin.get();
	
	srv.stop();
}

#endif


int main(int argc,char **argv)
{
	try 
	{
		settings par(argc,argv);	
		cppcms::impl::setup_logging(par.config);
#ifndef CPPCMS_WIN32
		main_posix(par);
#elif defined CPPCMS_WIN_NATIVE
		if(cppcms::impl::winservice::is_console(par.config))
			main_console(par);
		else
			main_win(par,argc,argv);
#else // cygwin
		main_cygwin(par);
#endif

	}
	catch(std::exception const &e) {
		BOOSTER_ERROR("cppcms_scale")<<e.what()<<booster::trace(e);
		return 1;
	}
	return 0;
}

