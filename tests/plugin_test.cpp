#include <cppcms/plugin.h>
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
	try {
		using cppcms::plugin::manager;
		booster::shared_object obj(path + "/" + booster::shared_object::name("plugin"));
		{
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
			TEST(manager::instance().entries("foo").size()==2);
			TEST(*manager::instance().entries("foo").begin()=="bar::create");
			TEST(*manager::instance().entries("foo").rbegin()=="lower");
		} 
		obj.close();
		std::cout << "- Unload" << std::endl;
		try {
			manager::instance().entry<std::string(std::string const &)>("foo","lower");
			std::cerr << "Must Not get there:" << __LINE__<<std::endl;
			return 1;
		}
		catch(cppcms::cppcms_error const &) {}
		catch(...) { throw std::runtime_error("Something else thrown"); }
		TEST(cppcms::plugin::manager::instance().has_plugin("foo")==false);
	}
	catch(std::exception const &e) {
		std::cerr << "Error:" << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
