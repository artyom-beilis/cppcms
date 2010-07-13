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
#include <cppcms/service.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/http_context.h>
#include <cppcms/url_dispatcher.h>
#include <booster/aio/deadline_timer.h>
#include <booster/posix_time.h>
#include <cppcms/json.h>
#include <iostream>
#include "client.h"

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : 
		cppcms::application(s),
		timer_(s.get_io_service())
	{
		dispatcher().assign("/normal",&unit_test::normal,this);
		dispatcher().assign("/throws",&unit_test::throws,this);
		dispatcher().assign("/delayed",&unit_test::delayed,this);
	}
	void normal()
	{
	}
	void throws()
	{
		throw std::runtime_error("Error");
	}
	void delayed()
	{
		context_ = release_context();
		timer_.expires_from_now(booster::ptime::milliseconds(300));
		timer_.async_wait(binder(this));
	}
	void handle()
	{
		context_->async_complete_response();
		context_.reset();
	}
private:
	struct binder {
		binder(booster::intrusive_ptr<unit_test> ptr) : self(ptr)
		{
		}
		void operator()(booster::system::error_code const &/*e*/) const
		{
			self->handle();
		}
		booster::intrusive_ptr<unit_test> self;
	};

	booster::aio::deadline_timer timer_;
	booster::shared_ptr<cppcms::http::context> context_;

};

int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		booster::intrusive_ptr<cppcms::application> app;
		if(!srv.settings().get("test.async",false)) {
			srv.applications_pool().mount( cppcms::applications_factory<unit_test>());
		}
		else {
			app=new unit_test(srv);
			srv.applications_pool().mount(app);
		}
		srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return run_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
