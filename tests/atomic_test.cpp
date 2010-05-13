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
