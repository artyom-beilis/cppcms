///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_TEST_RUN_CLIENT_H
#define CPPCMS_TEST_RUN_CLIENT_H

#include <stdlib.h>
#include <string>
#include <cppcms/service.h>
#include <cppcms/thread_pool.h>
#include <cppcms/json.h>

static bool run_ok=false;

struct runner {
	runner(cppcms::service &srv) : srv_(&srv)
	{
		command_ = srv.settings().get<std::string>("test.exec","none");
	}
	void operator()() const
	{
		if(command_ != "none") {
			run_ok = ::system(command_.c_str()) == 0;
			srv_->shutdown();	
		}
	}
private:
	cppcms::service *srv_;
	std::string command_;
};

struct submitter {
	submitter(cppcms::service &srv) : srv_(&srv) {}
	void operator()() const
	{
		srv_->thread_pool().post(runner(*srv_));
	}
	cppcms::service *srv_;
};


#endif
