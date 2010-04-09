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
#ifndef CPPCMS_TEST_RUN_CLIENT_H
#define CPPCMS_TEST_RUN_CLIENT_H

#include <stdlib.h>
#include <string>
#include "service.h"
#include "thread_pool.h"
#include "json.h"

static bool run_ok=false;

struct runner {
	runner(cppcms::service &srv) : srv_(&srv)
	{
		command_ = srv.settings().get<std::string>("test.exec");
	}
	void operator()() const
	{
		run_ok = ::system(command_.c_str()) == 0;
		srv_->shutdown();
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
