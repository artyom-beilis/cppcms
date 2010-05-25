#include <booster/aio/endpoint.h>
#include <booster/aio/socket.h>
#include <booster/aio/aio_config.h>
#include "test.h"
#include <iostream>

namespace io=booster::aio;
namespace sys=booster::system;

int main()
{
	try {
		io::endpoint ep("127.0.0.1",8080);
		TEST(ep.ip()=="127.0.0.1");
		TEST(ep.port() == 8080);
		ep.ip("192.168.2.100");
		TEST(ep.ip()=="192.168.2.100");
		ep.port(10);
		TEST(ep.port()==10);
		TEST(ep.family()==io::pf_inet);
		#ifndef BOOSTER_AIO_NO_PF_INET6
		ep.ip("::1");
		TEST(ep.family()==io::pf_inet6);
		#endif
		#ifndef BOOSTER_WIN32
		ep.path("/tmp/test");
		TEST(ep.path()=="/tmp/test");
		TEST(ep.family()==io::pf_unix);
		#endif
		ep=io::endpoint("127.0.0.1",8080);	
		io::socket a;
		a.open(io::pf_inet,io::sock_stream);
		a.set_option(io::socket::reuse_address,true);
		a.bind(ep);
		a.listen(1);

		io::socket s1;
		s1.open(io::pf_inet,io::sock_stream);
		s1.connect(ep);
		io::socket s2;
		a.accept(s2);
		io::endpoint rep = s2.remote_endpoint();
		TEST(rep.family()==io::pf_inet);
		TEST(rep.ip()=="127.0.0.1");

	}
	catch(std::exception const &e)
	{
		std::cerr<<"Failed:"<< e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
