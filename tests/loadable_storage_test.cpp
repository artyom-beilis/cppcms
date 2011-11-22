#include <cppcms/service.h>
#include <booster/aio/deadline_timer.h>
#include <iostream>

struct stopper {
	cppcms::service *srv;
	stopper(cppcms::service *s) : srv(s) {}
	void operator()(booster::system::error_code const &) const
	{
		srv->shutdown();
	}
};

int main(int argc,char **argv)
{

	try {
		cppcms::service srv(argc,argv);

		booster::aio::deadline_timer timer(srv.get_io_service());
		timer.expires_from_now(booster::ptime::seconds(1));
		timer.async_wait(stopper(&srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr<< "Failed: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
