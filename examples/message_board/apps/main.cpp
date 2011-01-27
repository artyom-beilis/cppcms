#include "mb.h"
#include <cppcms/service.h>
#include <cppcms/applications_pool.h>

#include <iostream>

int main(int argc,char ** argv)
{
	try {
		cppcms::service app(argc,argv);
		app.applications_pool().mount(cppcms::applications_factory<apps::mb>());
		app.run();
	}
	catch(std::exception const &e) {
		std::cerr<<e.what()<<std::endl;
	}
}

