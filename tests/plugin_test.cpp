#include <cppcms/plugin.h>
#include <booster/shared_object.h>
#include <iostream>
#include "test.h"


int main()
{
	try {
		booster::shared_object obj(booster::shared_object::name("plugin"));
		{
			booster::callback<std::string(std::string const &)> cb;
			cb = cppcms::plugin::manager::entry<std::string(std::string const &)>("foo::lower");
			TEST(cb("Hello World")=="hello world");
			try {
				cppcms::plugin::manager::entry<std::string(std::string &)>("foo::lower");
				TEST(!"Never Get There");
			}
			catch(booster::bad_cast const &) {}
			catch(...) { throw std::runtime_error("Something else thrown"); }
		} 
		obj.close();
		try {
			cppcms::plugin::manager::entry<std::string(std::string const &)>("foo::lower");
			std::cerr << "Must Not get there!" <<std::endl;
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
