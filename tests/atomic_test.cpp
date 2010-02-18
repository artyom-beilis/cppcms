#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/thread.hpp>
#else // Internal Boost
#   include <cppcms_boost/thread.hpp>
    namespace boost = cppcms_boost;
#endif
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
		boost::thread a(inc);
		boost::thread b(dec);
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
