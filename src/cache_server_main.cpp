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

struct params {
	bool en_cache;
	enum { none , files,sqlite3 } en_sessions;
	std::string session_backend;
	std::string session_file;
	std::string session_dir;
	int items_limit;
	int gc_frequency;
	int files_no;
	int port;
	std::string ip;
	int threads;

	void help()
	{
		std::cerr<<	
			"Usage cppcms_tcp_scale [parameter]\n"
			"    --bind IP          ipv4/ipv6 IPto bind (default 0.0.0.0)\n"
			"    --port N           port to bind -- MANDATORY\n"
			"    --threads N        number of threads, default 1\n"
			"    --cache            Enable cache module\n"
			"    --limit N          maximal Number of items to store\n"
			"                       mandatory if cache enabled\n"
			"    --session-files    Enable files bases session backend\n"
			"    --dir              Directory where files stored\n"
			"                       mandatory if session-files enabled\n"
			"    --gc N             gc frequencty seconds (default 600)\n"
			"                       it is enabled if threads > 1\n"
			"\n"
			"    At least one of   --session-files,"
			" --cache\n"
			"    should be defined\n"
			"\n";
	}
	params(int /*argc*/,char **argv) :
		en_cache(false),
		en_sessions(none),
		items_limit(-1),
		gc_frequency(-1),
		files_no(0),
		port(-1),
		ip("0.0.0.0"),
		threads(1)
	{
		using namespace std;
		argv++;
		while(*argv) {
			string param=*argv;
			char *next= *(argv+1);
			if(param=="--bind" && next) {
				ip=next;
				argv++;
			}
			else if(param=="--port" && next) {
				port=atoi(next);
				argv++;
			}
			else if(param=="--threads" && next) {
				threads=atoi(next);
				argv++;
			}
/*			else if(param=="--gc" && next) {
				gc_frequency=atoi(next);
				argv++;
			}*/
			else if(param=="--limit" && next) {
				items_limit=atoi(next);
				argv++;
			}
			else if(param=="--session-files") {
				en_sessions=files;
			}
			else if(param=="--dir" && next) {
				session_dir=next;
				argv++;
			}
			else if(param=="--cache") {
				en_cache=true;
			}
			else {
				help();
				throw runtime_error("Incorrect parameter:"+param);
			}
			argv++;
		}
		if(!en_cache && !en_sessions) {
			help();
			throw runtime_error("Neither cache nor sessions mods are defined");
		}
		if(en_sessions == files && session_dir.empty()) {
			help();
			throw runtime_error("parameter --dir undefined");
		}
		if(en_sessions == sqlite3 && session_file.empty()) {
			help();
			throw runtime_error("patameter --file undefined");
		}
		if(files_no == -1) files_no=1;
		if(port==-1) {
			help();
			throw runtime_error("parameter --port undefined");
		}
		if(en_cache && items_limit == -1) {
			help();
			throw runtime_error("parameter --limit undefined");
		}
		if(gc_frequency != -1) {
			if(threads == 1) {
				throw runtime_error("You have to use more then one thread to enable gc");
			}
		}
		if(threads > 1 && gc_frequency==-1) {
			gc_frequency = 600;
		}
	}
};


int main(int argc,char **argv)
{
	try 
	{
		params par(argc,argv);

		booster::intrusive_ptr<cppcms::impl::base_cache> cache;
		//auto_ptr<session_server_storage> storage;

		if(par.en_cache)
			cache = cppcms::impl::thread_cache_factory(par.items_limit);

		cppcms::impl::tcp_cache_service srv(cache,par.threads,par.ip,par.port);
		
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

