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
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_context.h>
#include <cppcms/json.h>
#include <iostream>
#include "client.h"
#include "test.h"

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
		dispatcher().assign("/ca",&unit_test::compressed_a,this);
		dispatcher().assign("/cb",&unit_test::compressed_b,this);
		dispatcher().assign("/bin",&unit_test::binary,this);
		dispatcher().assign("/not",&unit_test::not_compressed,this);
	}
	void compressed_a()
	{
		response().out()<< "test a";
	}
	void compressed_b()
	{
		response().set_plain_text_header();
		response().out()<< "test b";
	}
	void binary()
	{
		response().content_type("application/octet-stream");
		response().out()<< "binary";
	}
	void not_compressed()
	{
		response().io_mode(cppcms::http::response::nogzip);
		response().out() << "not compressed";
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
