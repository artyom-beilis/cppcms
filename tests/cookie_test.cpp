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
#include <cppcms/http_cookie.h>
#include <cppcms/http_response.h>
#include <cppcms/http_context.h>
#include <cppcms/json.h>
#include <iostream>
#include "client.h"
#include "test.h"

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string /*test*/)
	{
		response().set_cookie(cppcms::http::cookie("normal","token"));
		response().set_cookie(cppcms::http::cookie("utf","\xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D \xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D"));
		typedef cppcms::http::request::cookies_type cookies_type;
		cookies_type cookies=request().cookies();
		for(cookies_type::iterator cookie=cookies.begin();cookie!=cookies.end();cookie++) {
			response().out()<<cookie->second.name()<<':'<<cookie->second.value()<<'\n';
		}
	}
};


void basic_test()
{
	cppcms::http::cookie c("a","b");
	{
		std::ostringstream ss;
		ss << c;
		TEST(ss.str()=="Set-Cookie:a=b; Version=1");
	}
	{
		c.max_age(10);
		std::ostringstream ss;
		ss << c;
		TEST(ss.str()=="Set-Cookie:a=b; Max-Age=10; Version=1");
	}
	{
		c.expires(1);
		std::ostringstream ss;
		ss << c;
		TEST(ss.str()=="Set-Cookie:a=b; Max-Age=10; Expires=Thu, 01 Jan 1970 00:00:01 GMT; Version=1");
	}
	{
		std::ostringstream ss;
		c.browser_age();
		ss << c;
		TEST(ss.str()=="Set-Cookie:a=b; Version=1");
	}
	{
		c.expires(1);
		std::ostringstream ss;
		ss << c;
		TEST(ss.str()=="Set-Cookie:a=b; Expires=Thu, 01 Jan 1970 00:00:01 GMT; Version=1");
	}
}



int main(int argc,char **argv)
{
	try {
		basic_test();
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
