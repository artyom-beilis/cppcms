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
#include <cppcms/copy_filter.h>
#include <iostream>
#include "client.h"
#include "test.h"
#include <time.h>

int async_bad_count = 0;
int sync_bad_count = 0;
bool eof_detected = false;
int count_timeouts = 0;

int above_15 = 0;
int below_10 = 0;

bool write_tests = false;


class sync_test : public cppcms::application {
public:
	sync_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string /*unused*/)
	{
		bool bad_found = false;
		std::ostream &out = response().out();
		time_t start = time(0);
		for(unsigned i=0;i<10000000;i++) {
			if(!(out << i << '\n')) {
				bad_found = true;
				break;
			}
		}
		time_t end = time(0);
		if(end - start > 15)
			above_15 ++;
		else if(end - start < 10)
			below_10 ++;

		if(bad_found) {
			std::cout << "Disconned as expected" << std::endl;
			sync_bad_count++;
		}
		else {
			std::cout << "Not disconnected!" << std::endl;
		}
	}
};

class async_test : public cppcms::application {
public:
	async_test(cppcms::service &s) : cppcms::application(s)
	{
		dispatcher().assign("/goteof",&async_test::eof,this);
		dispatcher().assign("/long",&async_test::multiple,this);
	}
	struct eof_binder {
		booster::shared_ptr<cppcms::http::context> context;
		void operator()()
		{
			eof_detected = true;
		}
	};
	void eof()
	{
		eof_binder binder;
		binder.context = release_context();
		binder.context->async_on_peer_reset(binder);
	}
	struct binder {
		booster::shared_ptr<cppcms::http::context> context;
		int counter;
		time_t start;
		void operator()(cppcms::http::context::completion_type ct)
		{
			if(ct == cppcms::http::context::operation_aborted) {
				async_bad_count++;
				time_t end = time(0);
				if(end - start > 15)
					above_15 ++;
				else if(end - start < 10)
					below_10 ++;
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
		binder call;
		call.context = release_context();
		call.counter = 10000;
		call.start = time(0);
		call(cppcms::http::context::operation_completed);

	}
	
};

void print_count_report(std::ostream &out)
{
	out 	<< "Statistics:" << std::endl
		<< " sync_bad_count =" << sync_bad_count << std::endl
		<< " async_bad_count =" << async_bad_count << std::endl
		<< " count_timeouts =" << count_timeouts << std::endl
		<< " above 15 =" << above_15 << std::endl
		<< " below 10 =" << below_10 << std::endl
		<< " eof =" << eof_detected << std::endl;
}

int main(int argc,char **argv)
{
	std::string captured;
	try {
		cppcms::service srv(argc,argv);
		write_tests = srv.settings().get("test.write",false);
		booster::intrusive_ptr<cppcms::application> async = new async_test(srv);
		srv.applications_pool().mount( async, cppcms::mount_point("/async") );
		srv.applications_pool().mount( cppcms::applications_factory<sync_test>(), cppcms::mount_point("/sync"));
		srv.after_fork(submitter(srv));
		cppcms::copy_filter flt(std::cerr); // record the log
		srv.run();
		captured = flt.detach(); // get the log
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	size_t pos = 0;
	while((pos=captured.find("Timeout on connection",pos))!=std::string::npos) {
		pos+=10;
		count_timeouts++;
	}
	if(
		write_tests ?
		(
			async_bad_count != 2 
			|| sync_bad_count != 2 
			|| count_timeouts != 4
			|| above_15 != 2
			|| below_10 != 2
		)
		:
		(
			!eof_detected 
			|| count_timeouts != 5
		)
	  ) 
	{
		print_count_report(std::cerr);
		std::cerr << "Failed" << std::endl;
		return EXIT_FAILURE;
	}
	print_count_report(std::cout);
	if(!run_ok ) {
		std::cerr << "Python script failed" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Ok" << std::endl;
	return EXIT_SUCCESS;
}
