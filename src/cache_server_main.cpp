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
#endif
#include <iostream>
#include <stdlib.h>
#include <stdexcept>
#include <booster/shared_ptr.h>
#include <booster/intrusive_ptr.h>
#include <booster/shared_object.h>
#include <booster/thread.h>
#include "base_cache.h"
#include <cppcms/session_storage.h>
#include <cppcms/json.h>
#include <cppcms/service.h>
#include <cppcms/cppcms_error.h>

#ifdef CPPCMS_WIN_NATIVE
#include "session_win32_file_storage.h"
#else
#include "session_posix_file_storage.h"
#endif
#include "session_memory_storage.h"


struct settings {
	std::string ip;
	int port;
	int threads;
	int gc;
	
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
			cppcms::json::value v = cppcms::service::load_settings(argc,argv);
			setup(v);
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
				sessions.reset(new session_file_storage_factory(dir));
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

int main(int argc,char **argv)
{
	try 
	{
		settings par(argc,argv);

		cppcms::impl::tcp_cache_service srv(
			par.cache,
			par.sessions,
			par.threads,
			par.ip,
			par.port,
			par.gc);
		
#ifndef CPPCMS_WIN32
		// Wait for signlas for exit
		sigset_t wait_mask;
		sigemptyset(&wait_mask);
		sigaddset(&wait_mask, SIGINT);
		sigaddset(&wait_mask, SIGQUIT);
		sigaddset(&wait_mask, SIGTERM);
		pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
		int sig = 0;
		sigwait(&wait_mask, &sig);
		std::cout<<"Catched signal: exiting..."<<std::endl;
#else
		std::cout << "Press any key to stop..." << std::flush;
		std::cin.get();
#endif
		srv.stop();

	}
	catch(std::exception const &e) {
		std::cerr<<"Error:"<<e.what()<<std::endl;
		return 1;
	}
	std::cout<<"Done"<<std::endl;
	return 0;
}

