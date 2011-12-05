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
#include <cppcms/mount_point.h>
#include <cppcms/json.h>
#include <iostream>
#include "client.h"
#include "test.h"

int bad_count = 0;
int calls = 0;

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string /*unused*/)
	{
		calls ++;
		bool bad_found = false;
		std::ostream &out = response().out();
		for(unsigned i=0;i<10000000;i++) {
			if(!(out << i << '\n')) {
				bad_found = true;
				break;
			}
		}
		if(bad_found) {
			std::cout << "Disconned as expected" << std::endl;
			bad_count++;
		}
		else {
			std::cout << "Not disconnected!" << std::endl;
		}
	}
};

class async_unit_test : public cppcms::application {
public:
	async_unit_test(cppcms::service &s) : cppcms::application(s)
	{
		dispatcher().assign("/single",&async_unit_test::single,this);
		dispatcher().assign("/multiple",&async_unit_test::multiple,this);
	}
	struct binder {
		booster::shared_ptr<cppcms::http::context> context;
		int counter;
		void operator()(cppcms::http::context::completion_type ct)
		{
			if(ct == cppcms::http::context::operation_aborted) {
				bad_count++;
				return;
			}
			if(counter > 0) {
				counter --;
				std::ostream &out = context->response().out();
				for(unsigned i=0;i<1000;i++) {
					out << i << '\n';
				}
				context->async_flush_output(*this);
			}
			else {
				context->async_complete_response();
			}
		}
	};

	void multiple()
	{
		calls ++;
		binder call;
		call.context = release_context();
		call.counter = 10000;
		call(cppcms::http::context::operation_completed);

	}
	void single()
	{
		calls ++;
		std::ostream &out = response().out();
		for(unsigned i=0;i<100000;i++) {
			out << i << '\n';
		}
		release_context()->async_complete_response();
	}
	
};



int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		booster::intrusive_ptr<cppcms::application> async = new async_unit_test(srv);
		srv.applications_pool().mount( async, cppcms::mount_point("/async") );
		srv.applications_pool().mount( cppcms::applications_factory<unit_test>(), cppcms::mount_point("/sync"));
		srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	if(bad_count != 3 || calls != 4) {
		std::cerr << "Failed bad_count = " << bad_count << " calls = " << calls << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Ok" << std::endl;
	return EXIT_SUCCESS;
}
