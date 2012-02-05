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
				session().set("x","10");
				TEST(session().get<std::string>("x")=="10");
				TEST(session().get<int>("x")==10);
				TEST(session().get<double>("x")==10.0);
				TEST(session().get("z","default")=="default");
				TEST(session().get("x","default")=="10");
				TEST(session()["x"]=="10");
				TEST(session()["z"]=="");
				session()["z"]="test";
				TEST(session()["z"]=="test");

				mydata a,b;
				a.x=10;
				a.y=20;
				session().store_data("tmp",a);
				session().fetch_data("tmp",b);
				TEST(b.x==10);
				TEST(b.y==20);


				response().out() << "ok";
			}
			catch(std::exception const &e) {
				std::cerr << "Failed " << e.what() << std::endl;
				response().out() << "not ok";
			}
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
