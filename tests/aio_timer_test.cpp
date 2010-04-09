///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#include <service.h>
#include <application.h>
#include <applications_pool.h>
#include <localization.h>
#include <url_dispatcher.h>
#include <form.h>
#include <http_response.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include <aio_timer.h>
#include "test.h"

bool called = false;

class test : public cppcms::refcounted {
public:
	test(cppcms::service &srv) : 
		timer_(srv),
		srv_(srv)
	{	
		std::cout << "Testing expires_from_now" << std::endl;
		now_ = time(0);
		timer_.expires_from_now(1);
		timer_.async_wait(wrapper(this,&test::test1));
	}
	void test1(bool err_or_can)
	{
		std::cout << "Testing expires_at" << std::endl;
		TEST(!err_or_can);
		TEST(time(0) - now_ >= 1);
		timer_.expires_at(time(0)+2);
		timer_.async_wait(wrapper(this,&test::test2));
	}
	void test2(bool err_or_can)
	{
		std::cout << "Testing cancel" << std::endl;
		TEST(!err_or_can);
		TEST(time(0) - now_ >= 3);
		TEST(time(0) - now_ < 5);
		timer_.expires_at(time(0)+2);
		timer_.async_wait(wrapper(this,&test::test3));
		timer_.cancel();
	}
	void test3(bool err_or_can)
	{
		TEST(err_or_can);
		srv_.shutdown();
		called = true;
	}
private:
	struct wrapper {
		wrapper(cppcms::intrusive_ptr<test> p,void (test::*member)(bool)) :
			p_(p),
			member_(member)
		{
		}
		void operator()(bool err_or_can) const
		{
			((*p_).*member_)(err_or_can);
		}
	private:
		cppcms::intrusive_ptr<test> p_;
		void (test::*member_)(bool);
	};
	time_t now_;
	cppcms::aio::timer timer_;
	cppcms::service &srv_;
};



int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		cppcms::intrusive_ptr<test> t=new test(srv);
		srv.run();
		TEST(called);
	}
	catch(std::exception const &e) {
		std::cerr << "Failed: "<< e.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Ok" << std::endl;
	return EXIT_SUCCESS;
}
