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
#include <cppcms/http_cookie.h>
#include <cppcms/session_interface.h>
#include <cppcms/serialization.h>
#include <cppcms/json.h>
#include <iostream>
#include "client.h"
#include "test.h"

struct mydata : public cppcms::serializable { 
	int x;
	int y;
	void serialize(cppcms::archive &a)
	{
		a & x & y;
	}
};

void test_set(cppcms::session_interface &interface,std::string const &values)
{
	std::set<std::string> keys = interface.key_set();
	std::string res;
	for(std::set<std::string>::const_iterator p=keys.begin();p!=keys.end();++p) {
		if(res.empty())
			res = *p;
		else
			res += "|" + *p;
	}
	if(res!=values)
		throw std::runtime_error("Key set is invalid:(" + res +")!=(" + values +")");
}

class adapter : public cppcms::session_interface_cookie_adapter {
public:
	adapter(cppcms::http::context &c) 
	{
		cookie_name = c.session().session_cookie_name();
		value = c.request().get("sid");
	}
	
	void set_cookie(cppcms::http::cookie const &updated_cookie)
	{
		if(updated_cookie.name() == cookie_name) {
			if((updated_cookie.max_age_defined() && updated_cookie.max_age() == 0)
			   || (updated_cookie.expires_defined() && updated_cookie.expires() + 1 < time(0)))
				value.clear();
			else
				value = updated_cookie.value();
		}
	}
	std::string get_session_cookie(std::string const &/*name*/)
	{
		return value;
	}
	std::set<std::string> get_cookie_names() { return std::set<std::string>(); }
	std::string value;
	std::string cookie_name;
};

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string u)
	{
		if(u=="/new") {
			session().set("x",1);
		}
		else if(u=="/update") {
			session().set("x",session().get<int>("x") + 1);
		}
		else if(u=="/clear") {
			session().clear();
			response().out() << "clear";
		}
		else if(u=="/new_short") {
			session().set("x",1);
			session().age(1);
		}
		else if(u=="/is_expired") {
			if(session().is_set("x"))
				response().out() << "not expired";
			else
				response().out() << "expired";
		}
		else if(u=="/expose") {
			session().expose("x");
		}
		else if(u=="/unexpose") {
			session().hide("x");
		}
		else if(u=="/fixed") {
			session().expiration(cppcms::session_interface::fixed);
		}
		else if(u=="/renew") {
			session().expiration(cppcms::session_interface::renew);
		}
		else if(u=="/browser") {
			session().expiration(cppcms::session_interface::browser);
		}
		else if(u=="/reset") {
			session().reset_session();
		}
		else if(u=="/on_server") {
			session().on_server(true);
		}
		else if(u=="/not_on_server") {
			session().on_server(false);
		}
		else if(u=="/huge") {
			std::string tmp="x";
			for(int i=0;i<10;i++)
				tmp=tmp+tmp;
			session().set("y",tmp);
		}
		else if(u=="/small") {
			session().erase("y");
			session().set("x","1");
		}
		else if(u=="/info") {
			bool is_set_x = session().is_set("x");
			bool is_exposed_x = session().is_exposed("x");
			int age = session().age();
			int expiration = session().expiration();
			bool on_server = session().on_server();
			response().out() 
				<< "is_set_x=" << is_set_x << "\n"
				<< "is_exposed_x=" << is_exposed_x << "\n"
				<< "age="<<age<<"\n"
				<< "expiration=" << expiration << "\n"
				<< "on_server=" << on_server ;
		}
		else if(u=="/api") {
			try {
				// Fix Me Later
				test_set(session(),"");
				session().set("x","10");
				test_set(session(),"x");
				TEST(session().get<std::string>("x")=="10");
				TEST(session().get<int>("x")==10);
				TEST(session().get<double>("x")==10.0);
				TEST(session().get("z","default")=="default");
				TEST(session().get("x","default")=="10");
				TEST(session()["x"]=="10");
				TEST(session()["z"]=="");
				test_set(session(),"x|z");
				session()["z"]="test";
				TEST(session()["z"]=="test");

				mydata a,b;
				a.x=10;
				a.y=20;
				session().store_data("tmp",a);
				session().fetch_data("tmp",b);
				test_set(session(),"tmp|x|z");
				TEST(b.x==10);
				TEST(b.y==20);


				response().out() << "ok";
			}
			catch(std::exception const &e) {
				std::cerr << "Failed " << e.what() << std::endl;
				response().out() << "not ok";
			}
		}
		else if(u=="/alt") {
			adapter a(context());
			session().set_cookie_adapter_and_reload(a);
			bool is_set=session().is_set("x");
			std::string val;
			if(is_set)
				val=session().get("x");
			if(request().get("x")!="")
				session().set("x",request().get("x"));
			response().out() << "is_set_x="<<is_set << '\n'
					 << "x='" << val << "'\n";
			response().out() << "sid=" << a.value;
		}
	}
};





int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		srv.applications_pool().mount( cppcms::create_pool<unit_test>());
		srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return run_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
