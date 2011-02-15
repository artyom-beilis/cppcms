//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/posix_time.h>
#include <booster/ctime.h>
#include "test.h"

#include <limits>
#include <iostream>

void test_lower(booster::ptime l,booster::ptime h)
{
	TEST(l<h);
	TEST(l!=h);
	TEST(h>l);
	TEST(h!=l);
	TEST(l<=h);
	TEST(h>=l);
}

int main()
{
	try {
		using booster::ptime;

		std::cout << "- Testing ptime" << std::endl;
		ptime p;
		TEST(p==ptime::zero);
		TEST(p.get_seconds()==0 && p.get_nanoseconds()==0);
		p=ptime(10,20);
		TEST(p==ptime(10,20));
		TEST(p.get_seconds()==10 && p.get_nanoseconds()==20);
		
		p=ptime(15);
		TEST(p==ptime(15,0));
		TEST(p.get_seconds()==15 && p.get_nanoseconds()==0);


		p=ptime::milliseconds(2100);
		TEST(p.get_seconds()==2 && p.get_nanoseconds() == 100000000);
		TEST(ptime::milliseconds(p)==2100);
		TEST(p.get_milliseconds()==100);
		p=ptime::microseconds(3230000);
		TEST(p.get_seconds() == 3 && p.get_nanoseconds() == 230000000);
		TEST(ptime::microseconds(p)==3230000);
		TEST(p.get_microseconds()==230000);
		p=ptime::nanoseconds(1000000100);
		TEST(p.get_seconds()==1 && p.get_nanoseconds() == 100);
		p=ptime::seconds(15);
		TEST(p==ptime(15) && ptime::seconds(p)==15);
		p=ptime::minutes(15);
		TEST(p==ptime(15*60) && ptime::minutes(p)==15);
		p=ptime::hours(15);
		TEST(p==ptime(15*3600) && ptime::hours(p)==15);
		p=ptime::days(15);
		TEST(p==ptime(15*3600*24) && ptime::days(p)==15);
		
		p=ptime(100,1000000001);
		TEST(p.get_seconds()==101 && p.get_nanoseconds()==1);
		
		p=ptime(100,2000000001);
		TEST(p.get_seconds()==102 && p.get_nanoseconds()==1);

		p=ptime(100,2000000000);
		TEST(p.get_seconds()==102 && p.get_nanoseconds()==0);
		
		p=ptime(100,-999999999);
		TEST(p.get_seconds()==99 && p.get_nanoseconds()==1);
		
		p=ptime(100,-1999999998);
		TEST(p.get_seconds()==98 && p.get_nanoseconds()==2);

		TEST(ptime(100,0)!=ptime(101,0));
		TEST(ptime(100,1)!=ptime(100,0));
		TEST(ptime(101,1)!=ptime(100,0));
		TEST(ptime(100,3)==ptime(100,3));
		TEST(ptime(100,3)<=ptime(100,3));
		TEST(ptime(100,3)>=ptime(100,3));

		TEST(ptime(100,2)+ptime(101,3)==ptime(201,5));
		TEST(ptime(100,2)+ptime(1,999999999)==ptime(102,1));
		TEST(ptime(100,0)-ptime(1,1)==ptime(98,999999999));
		TEST(ptime(100,0)-ptime(0,999999999)==ptime(99,1));
		TEST(ptime(100,10)-ptime(1,5)==ptime(99,5));

		test_lower(ptime(100,0),ptime(101,0));
		test_lower(ptime(100,0),ptime(100,1));
		p=ptime(10,100000000);
		TEST(10.099999 < ptime::to_number(p) && ptime::to_number(p) < 10.10001);
		TEST(ptime(10, 9999990) < ptime::from_number(10.1) && ptime::from_number(10.1) <ptime(10,100000010));

		ptime start=ptime::now();
		ptime::sleep(ptime::milliseconds(100));
		ptime end = ptime::now();
		int diff = ptime::milliseconds(end-start);
		TEST(100 <= diff && diff <=300);

		std::cout << "- Testing ctime" << std::endl;
		std::cout << "- Testing timegm normalization" << std::endl;	
	
		std::tm src = std::tm();
		src.tm_year = 2011 - 1900;
		src.tm_mday = 14;
		src.tm_mon = 1; // feb

		{
			std::tm mon=src;
			mon.tm_mon+=13;
			booster::normalize_universal_time(mon);
			TEST(mon.tm_mon == 2);
			TEST(mon.tm_year == 2012 - 1900);
			TEST(mon.tm_mday == 14);
		}
		{
			std::tm mon=src;
			mon.tm_mon-=24;
			booster::normalize_universal_time(mon);
			TEST(mon.tm_mon == 1);
			TEST(mon.tm_year == 2009 - 1900);
			TEST(mon.tm_mday == 14);
		}
		{
			std::tm mon=src;
			mon.tm_mday+=15;
			booster::normalize_universal_time(mon);
			TEST(mon.tm_mon == 2);
			TEST(mon.tm_year == 2011 - 1900);
			TEST(mon.tm_mday == 1);
		}
		{
			std::tm mon=src;
			mon.tm_year = 2000 - 1900;
			mon.tm_mday+=15;
			booster::normalize_universal_time(mon);
			TEST(mon.tm_mon == 1);
			TEST(mon.tm_year == 2000 - 1900);
			TEST(mon.tm_mday == 29);
		}
	
		std::cout << "- Testing timegm implementation" << std::endl;	
		long long range = 500LL * 3600 * 24 * 366;
		if(range > std::numeric_limits<time_t>::max()) {
			range = std::numeric_limits<time_t>::max() - 3600 * 24;
		}
#ifdef BOOSTER_WIN_NATIVE
		// Under Windows localtime and gmtime are valid for positive time_t values
		// only
		long long low_point = 0;
#else
		long long low_point = -range;
#endif
		long long top_point = +range;

		for(time_t pt = low_point;pt < top_point;pt+=3600 * 12) {
			// One is system other is mine
			std::tm t_ref = booster::universal_time(pt);
			TEST(booster::make_universal_time(booster::universal_time(pt)) == pt);
			std::tm t_cpy = std::tm();
			t_cpy.tm_year = t_ref.tm_year;
			t_cpy.tm_mon = t_ref.tm_mon;
			t_cpy.tm_mday = t_ref.tm_mday;
			t_cpy.tm_hour = t_ref.tm_hour;
			t_cpy.tm_min = t_ref.tm_min;
			t_cpy.tm_sec = t_ref.tm_sec;
			t_cpy.tm_isdst = -1;
			TEST(booster::normalize_universal_time(t_cpy) == pt);
			TEST(t_cpy.tm_wday == t_ref.tm_wday);
			TEST(t_cpy.tm_yday == t_ref.tm_yday);
		}



	}
	catch(std::exception const &e) {
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
}

