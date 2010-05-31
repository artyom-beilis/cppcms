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

#include <cppcms/locale_fwd.h>

namespace booster {
	namespace aio {
		class io_service;
	}
}


namespace cppcms {
	namespace impl {
		class service;
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

	class CPPCMS_API service : public booster::noncopyable
	{
	public:
		service(json::value const &v);
		service(int argc,char *argv[]);
		~service();

		void run();
		void shutdown();

		json::value const &settings();
		
		cppcms::applications_pool &applications_pool();
		cppcms::thread_pool &thread_pool();
		cppcms::session_pool &session_pool();
		cppcms::views_pool &views_pool();
		cppcms::cache_pool &cache_pool();
		cppcms::forwarder &forwarder();

		locale::generator const &generator();
		std::locale locale();
		std::locale locale(std::string const &name);


		booster::aio::io_service &get_io_service();

		cppcms::impl::service &impl();

		void post(booster::function<void()> const &handler);
		void after_fork(booster::function<void()> const &handler);
		
		int threads_no();
		int procs_no();
		int process_id();
	private:
		void setup();
		void setup_logging();
		void setup_acceptor(json::value const &,int);
		void load_settings(int argc,char *argv[]);
		void stop();
		void start_acceptor();
		void setup_exit_handling();
		bool prefork();
		booster::hold_ptr<impl::service> impl_;
	};

} //




#endif
