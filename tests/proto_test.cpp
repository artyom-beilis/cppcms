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

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string /*unused*/)
	{
		response().set_plain_text_header();
		TEST(is_async == is_asynchronous());
		if(!is_asynchronous()) {
			TEST(response().io_mode() == cppcms::http::response::normal);
			response().io_mode(cppcms::http::response::nogzip);
		}
		else {
			TEST(response().io_mode() == cppcms::http::response::asynchronous);
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
		booster::intrusive_ptr<cppcms::application> app;
		if(srv.settings().get("test.async","sync")=="sync") {
			std::cout << "Synchronous testing" << std::endl;
			srv.applications_pool().mount( cppcms::applications_factory<unit_test>());
		}
		else {
			is_async = true;
			std::cout << "Asynchronous testing" << std::endl;
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
