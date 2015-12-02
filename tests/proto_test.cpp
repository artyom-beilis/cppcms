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
#include <cppcms/json.h>
#include <iostream>
#include "client.h"
#include "test.h"

bool is_async;
bool is_nonblocking;

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string /*unused*/)
	{
		response().set_plain_text_header();
		response().setbuf(64);
		TEST(is_async == is_asynchronous());
		if(!is_asynchronous()) {
			TEST(response().io_mode() == cppcms::http::response::normal);
			response().io_mode(cppcms::http::response::nogzip);
		}
		else {
			TEST(response().io_mode() == cppcms::http::response::asynchronous);
		}
		if(is_nonblocking) {
			response().full_asynchronous_buffering(false);
		}
		std::map<std::string,std::string> env=request().getenv();
		std::ostream &out = response().out();
		for(std::map<std::string,std::string>::const_iterator p=env.begin();p!=env.end();++p) {
			out << p->first <<':'<<p->second << '\n';
		}
		out << '\n';
		typedef cppcms::http::request::form_type form_type;
		form_type const &form=request().post();
		for(form_type::const_iterator p=form.begin();p!=form.end();++p) {
			out << p->first <<'='<<p->second << '\n';
		}
	}
};





int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		if(srv.settings().get("test.async","sync")=="sync") {
			std::cout << "Synchronous testing" << std::endl;
			srv.applications_pool().mount( cppcms::create_pool<unit_test>());
		}
		else {
			if(srv.settings().get<std::string>("test.async")=="async") {
				is_async = true;
				std::cout << "Asynchronous testing" << std::endl;
			}
			else if(srv.settings().get<std::string>("test.async")=="nonblocking") {
				is_async = true;
				is_nonblocking = true;
				std::cout << "Non blocking testing" << std::endl;
			}
			else {
				std::cerr << "Invalid configuration value of test.async" << std::endl;
				return 1;
			}
			srv.applications_pool().mount( cppcms::create_pool<unit_test>(),cppcms::app::asynchronous);
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
