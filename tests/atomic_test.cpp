///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/config.h>
#include <booster/thread.h>
#include "atomic_counter.h"
#include "test.h"
#include <iostream>

cppcms::atomic_counter *c;

void inc()
{
	for(int i=0;i<1000000;i++)
		++*c;
}

void dec()
{
	for(int i=0;i<1000000;i++)
		--*c;
}



int main()
{
	try {
		cppcms::atomic_counter counter(0);
		TEST(long(counter) == 0);
		TEST(++counter == 1);
		TEST(--counter == 0);
		TEST(++counter == 1);
		TEST(long(counter) == 1);
		TEST(++counter == 2);
		TEST(--counter == 1);
		TEST(long(counter) == 1);
		TEST(--counter == 0);
		TEST(long(counter) == 0);
		c=&counter;
		booster::thread a(inc);
		booster::thread b(dec);
		a.join();
		b.join();
		TEST(long(counter) == 0);
	}
	catch(std::exception const &e) {
		std::cerr << "Failed " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;

}
