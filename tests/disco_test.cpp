///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
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
		std::cout << "Running sync test\n" << std::endl;
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
			std::cout << "Sync Disconned as expected" << std::endl;
			bad_count++;
		}
		else {
			std::cout << "Sync Not disconnected!" << std::endl;
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
			std::cout << "Async IO Completed " << std::endl;
			if(ct == cppcms::http::context::operation_aborted) {
				std::cout << "Async Disconned as expected" << std::endl;
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
				std::cout << "Async Not disconnected!" << std::endl;
				context->async_complete_response();
			}
		}
	};

	void multiple()
	{
		std::cout << "Running async test\n" << std::endl;
		calls ++;
		binder call;
		call.context = release_context();
		call.counter = 10000;
		call(cppcms::http::context::operation_completed);

	}
	void single()
	{
		std::cout << "Running async test without error reporting\n" << std::endl;
		calls ++;
		std::ostream &out = response().out();
		for(unsigned i=0;i<100000;i++) {
			out << i << '\n';
		}
		release_context()->async_complete_response();
	}
	
};

class nonblocking_unit_test : public cppcms::application {
public:
	nonblocking_unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	struct binder : public booster::callable<void(cppcms::http::context::completion_type)> {
		typedef booster::intrusive_ptr<binder> self_ptr;
		booster::shared_ptr<cppcms::http::context> context;
		int counter;
		binder(booster::shared_ptr<cppcms::http::context> ctx) :
			context(ctx),
			counter(0)
		{
		}
		void run()
		{
			(*this)(cppcms::http::context::operation_completed);
		}
		void operator()(cppcms::http::context::completion_type ct)
		{
			if(ct == cppcms::http::context::operation_aborted) {
				std::cout << "Nonblocking Error on completion detected" << std::endl;
				bad_count++;
				return;
			}
			std::ostream &out = context->response().out();
			for(;counter < 100000;counter++) {
				out << counter << '\n';
				if(!out) {
					std::cout << "Nonblocking Error on stream detected" << std::endl;
					bad_count++;
					return;
				}
				if(context->response().pending_blocked_output()) {
					std::cout << "Nonblocking Got blocking status at" << counter << std::endl;
					context->async_flush_output(self_ptr(this));
					return;
				}
			}
			std::cout << "Nonblocking No error detected" << std::endl;
			context->async_complete_response();
		}
	};

	void main(std::string)
	{
		std::cout << "Running non-blocking test" << std::endl;
		response().setbuf(0);
		response().full_asynchronous_buffering(false);
		calls ++;
		binder::self_ptr p(new binder(release_context()));
		p->run();
	}
};



int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		srv.applications_pool().mount( 
			cppcms::create_pool<async_unit_test>(),
			cppcms::mount_point("/async"),
			cppcms::app::asynchronous);

		srv.applications_pool().mount(
			cppcms::create_pool<nonblocking_unit_test>(),
			cppcms::mount_point("/nonblocking"),
			cppcms::app::asynchronous);

		srv.applications_pool().mount( cppcms::create_pool<unit_test>(), cppcms::mount_point("/sync"));
		srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	if(bad_count != 4 || calls != 5) {
		std::cerr << "Failed bad_count = " << bad_count << " (exp 4) calls = " << calls << " (exp 5)"<< std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Ok" << std::endl;
	return EXIT_SUCCESS;
}
