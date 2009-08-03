#include "application.h"
#include "url_dispatcher.h"
#include "applications_pool.h"
#include "service.h"
#include "http_response.h"


class hello : public cppcms::application {
public:
	hello(cppcms::service &srv) : 
		cppcms::application(srv)
	{
		dispatcher().assign(".*",&hello::hello_world,this);
	}
	void hello_world()
	{
		response().out() <<
			"<html><body>\n"
			"<h1>Hello World!</h1>\n"
			"<body></html>\n";
	}
};


int main(int argc,char **argv)
{
	try {
		cppcms::service service(argc,argv);
		service.applications_pool().mount(".*",cppcms::applications_factory<hello>());
		service.run();
	}
	catch(std::exception const &e) {
		std::cerr<<e.what()<<std::endl;
		return 1;
	}
	return 0;
}
