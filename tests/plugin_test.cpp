#include <cppcms/plugin.h>
#include <booster/shared_object.h>
#include <iostream>
#include "test.h"


int main(int argc,char **argv)
{
	if(argc!=2) {
		std::cerr << "Usage plugin_test /path/to/the/plugin/dir" << std::endl;
		return 1;
	}
	std::string path = argv[1];
	try {
		booster::shared_object obj(path + "/" + booster::shared_object::name("plugin"));
		{
			std::cout << "- Normal call" << std::endl;
			booster::callback<std::string(std::string const &)> cb;
			cb = cppcms::plugin::manager::entry<std::string(std::string const &)>("foo::lower");
			TEST(cb("Hello World")=="hello world");
			try {
				std::cout << "- Bad signature" << std::endl;
				cppcms::plugin::manager::entry<std::string(std::string &)>("foo::lower");
				TEST(!"Never Get There");
			}
			catch(booster::bad_cast const &) {}
			catch(...) { throw std::runtime_error("Something else thrown"); }
		} 
		obj.close();
		std::cout << "- Unload" << std::endl;
		try {
			cppcms::plugin::manager::entry<std::string(std::string const &)>("foo::lower");
			std::cerr << "Must Not get there:" << __LINE__<<std::endl;
			return 1;
		}
		catch(cppcms::cppcms_error const &) {}
		catch(...) { throw std::runtime_error("Something else thrown"); }
	}
	catch(std::exception const &e) {
		std::cerr << "Error:" << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
