#include "test.h"
#include <booster/aio/deadline_timer.h>
#include <booster/aio/aio_category.h>
#include <booster/system_error.h>
#include <booster/aio/io_service.h>

#include <iostream>

booster::ptime occured1_at;
booster::ptime occured2_at;

struct time1 {
	booster::aio::io_service *srv;
	booster::aio::deadline_timer *timer2;
	void operator()(booster::system::error_code const &e)
	{
		TEST(!e);
		occured1_at = booster::ptime::now();
		timer2->cancel();
	}
};

struct time2 {
	booster::aio::io_service *srv;
	void operator()(booster::system::error_code const &e)
	{
		TEST(e.value()==booster::aio::aio_error::canceled);
		occured2_at = booster::ptime::now();
		srv->stop();

	}
};



int main()
{
	try {
		using booster::ptime;
		booster::aio::deadline_timer timer;
		booster::ptime now=booster::ptime::now(),tmp;
		timer.expires_at(now + ptime::seconds(1));
		TEST(timer.expires_at() == now + ptime::seconds(1));
		TEST((tmp = timer.expires_from_now()) <= ptime::milliseconds(1000) && tmp >=ptime::milliseconds(900));
		timer.expires_from_now(ptime::milliseconds(2000));
		TEST((tmp = timer.expires_at()) >= now + ptime::seconds(2));
		TEST(tmp <= now + ptime::milliseconds(2100));
		TEST((tmp = timer.expires_from_now()) <= ptime::seconds(2) && tmp >= ptime::milliseconds(1900));
		
		now = booster::ptime::now();
		timer.expires_from_now(ptime::milliseconds(100));
		timer.wait();
		TEST( (tmp = (booster::ptime::now() - now)) >= ptime::milliseconds(100));
		TEST( tmp < ptime::milliseconds(300));

		booster::aio::io_service srv;
		timer.set_io_service(srv);
		booster::aio::deadline_timer t2(srv);
		
		now = booster::ptime::now();


		t2.expires_from_now(ptime::seconds(2));
		timer.expires_from_now(ptime::milliseconds(100));
		time1 exec1 = { &srv, &t2 };
		time2 exec2 = { &srv };
		timer.async_wait(exec1);
		t2.async_wait(exec2);
		srv.run();
		TEST(occured2_at >= occured1_at);
		TEST(occured1_at >= now + ptime::milliseconds(100));
		TEST(occured2_at < now + ptime::milliseconds(300));
	}
	catch(std::exception const &e)
	{
		std::cerr << "Fail: " <<e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
