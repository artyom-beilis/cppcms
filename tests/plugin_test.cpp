#include <cppcms/plugin.h>
#include <cppcms/service.h>
#include <cppcms/json.h>
#include <booster/shared_object.h>
#include <iostream>
#include "test.h"
#include "plugin_base.h"


int main(int argc,char **argv)
{
	if(argc!=2) {
		std::cerr << "Usage plugin_test /path/to/the/plugin/dir" << std::endl;
		return 1;
	}
	std::string path = argv[1];
	cppcms::json::value params;

	params["plugin"]["paths"][0]=path;
	params["plugin"]["modules"][0]="plugin";

	try {
		using cppcms::plugin::manager;
		{
			cppcms::plugin::scope sc(params);
			TEST(sc.is_loaded_by_this_scope("plugin"));
			TEST(cppcms::plugin::scope::is_loaded("plugin"));

			std::cout << "- Normal call" << std::endl;
			booster::callback<std::string(std::string const &)> cb;
			cb = manager::instance().entry<std::string(std::string const &)>("foo","lower");
			TEST(cb("Hello World")=="hello world");
			bar_base *b=manager::instance().entry<bar_base *(std::string const &)>("foo","bar::create")("hello");
			TEST(b->msg() == std::string("hello"));
			delete b;
			try {
				std::cout << "- Bad signature" << std::endl;
				manager::instance().entry<std::string(std::string &)>("foo","lower");
				TEST(!"Never Get There");
			}
			catch(cppcms::plugin::signature_error const &e) {
				std::string msg = e.what();
				TEST(msg.find("std::string(std::string const &)")!=std::string::npos);
			}
			catch(...) { throw std::runtime_error("Something else thrown"); }
			std::cout << "- Iteration" << std::endl;
			TEST(manager::instance().has_plugin("foo"));
			TEST(manager::instance().plugins().size()==1);
			TEST(*manager::instance().plugins().begin()=="foo");
			TEST(manager::instance().entries("foo").size()==3);
			std::set<std::string> names = manager::instance().entries("foo");
			std::set<std::string>::iterator p=names.begin();
			TEST(*p++ == "bar::create");
			TEST(*p++ == "counter");
			TEST(*p++ == "lower");
			TEST(p==names.end());
			TEST(manager::instance().entry<int()>("foo","counter")()==1);
			TEST(manager::instance().entry<int()>("foo","counter")()==2);
		} 
		std::cout << "- Unload" << std::endl;
		TEST(!cppcms::plugin::scope::is_loaded("plugin"));
		try {
			manager::instance().entry<std::string(std::string const &)>("foo","lower");
			std::cerr << "Must Not get there:" << __LINE__<<std::endl;
			return 1;
		}
		catch(cppcms::cppcms_error const &) {}
		catch(...) { throw std::runtime_error("Something else thrown"); }
		TEST(cppcms::plugin::manager::instance().has_plugin("foo")==false);
		std::cout << "- Scope vs Service" << std::endl;
		{
			cppcms::service srv(params);
			TEST(manager::instance().has_plugin("foo"));
			TEST(manager::instance().entry<int()>("foo","counter")()==1);
			TEST(manager::instance().entry<int()>("foo","counter")()==2);
			TEST(srv.plugins().is_loaded_by_this_scope("plugin"));
			TEST(cppcms::plugin::scope::is_loaded("plugin"));
			{
				cppcms::plugin::scope sc(params);
				TEST(!sc.is_loaded_by_this_scope("plugin"));
				TEST(manager::instance().entry<int()>("foo","counter")()==3);
			}
			TEST(manager::instance().entry<int()>("foo","counter")()==4);
		}
		{
			cppcms::plugin::scope sc(params);
			TEST(manager::instance().has_plugin("foo"));
			TEST(manager::instance().entry<int()>("foo","counter")()==1);
			TEST(manager::instance().entry<int()>("foo","counter")()==2);
			TEST(cppcms::plugin::scope::is_loaded("plugin"));
			{
				cppcms::service srv(params);
				TEST(!srv.plugins().is_loaded_by_this_scope("plugin"));
				TEST(sc.is_loaded_by_this_scope("plugin"));
				TEST(manager::instance().entry<int()>("foo","counter")()==3);
			}
			TEST(manager::instance().entry<int()>("foo","counter")()==4);
		}
		TEST(!manager::instance().has_plugin("foo"));
		TEST(!cppcms::plugin::scope::is_loaded("plugin"));
	}
	catch(std::exception const &e) {
		std::cerr << "Error:" << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
