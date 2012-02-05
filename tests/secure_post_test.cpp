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
#include <cppcms/session_interface.h>
#include <cppcms/serialization.h>
#include <cppcms/json.h>
#include <cppcms/form.h>
#include <iostream>
#include "client.h"
#include "test.h"


class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string u)
	{
		if(u=="/gettoken") {
			session().set("x",1);
			response().out();
			response().out() << session().get_csrf_token();
		}
		else if(u=="/post") {
			cppcms::form frm;
			cppcms::widgets::text txt;
			cppcms::widgets::file fl;
			txt.name("test");
			fl.name("file");
			frm.add(txt);
			frm.add(fl);
			try {
				frm.load(context());
				response().out() << "ok";
			}
			catch(cppcms::request_forgery_error const &e) {
				response().out() << "fail";
			}
		}
		else {
			response().out() << "not there";
		}
	}
};





int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		srv.applications_pool().mount( cppcms::applications_factory<unit_test>());
		srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return run_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
