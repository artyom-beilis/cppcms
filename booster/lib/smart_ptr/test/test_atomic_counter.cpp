//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/config.h>
#include <booster/thread.h>
#include <booster/atomic_counter.h>
#include "test.h"
#include <iostream>

booster::atomic_counter *c;

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
		booster::atomic_counter counter(0);
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
