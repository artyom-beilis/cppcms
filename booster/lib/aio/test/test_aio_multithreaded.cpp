#include <booster/aio/socket.h>
#include <booster/aio/buffer.h>
#include <booster/aio/io_service.h>
#include <booster/aio/deadline_timer.h>
#include <booster/system_error.h>
#include <iostream>
#include "test.h"

#ifndef BOOSTER_WIN32
#include <unistd.h>
#endif

namespace io=booster::aio;
namespace sys=booster::system;

void make_pair(io::socket &s1,io::socket &s2)
{
	io::socket a;
	a.open(io::pf_inet,io::sock_stream);
	a.set_option(io::socket::reuse_address,true);
	a.bind(io::endpoint("127.0.0.1",0));
	a.listen(1);
	s1.open(io::pf_inet,io::sock_stream);
	s1.connect(a.local_endpoint());
	a.accept(s2);
	a.close();
}


struct service {
	io::socket *self;
	bool reader;
	void reader
};


int main()
{
	try {
		test_buffer();
		basic_io();
		readv_writev();
		get_set_srv();
		test_async_connect();
		test_unix();
		test_pair();
		test_async_send_recv();
		test_cancel_operations();
		test_full_read_write();
		test_full_async_read_write();
	}

	catch(std::exception const &e)
	{
		std::cout<<"Failed:"<< e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
