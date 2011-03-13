//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/ctime.h>
#include <booster/backtrace.h>
#include <booster/config.h>
#ifdef BOOSTER_MSVC
#  pragma warning(disable : 4996)
#endif

namespace booster {

std::tm local_time(time_t pt)
{
	#ifdef BOOSTER_WIN_NATIVE
	// WIN32 Uses TSL
	std::tm *res=::localtime(&pt);
	if(!res)
		throw runtime_error("booster::local_time: Failed to convert time to local time");
	return *res;
	#else
	std::tm tm;
	if(!::localtime_r(&pt,&tm)) {
		throw runtime_error("booster::local_time: Failed to convert time to local time");
	}
	return tm;
	#endif
}
std::tm universal_time(time_t pt)
{
	#ifdef BOOSTER_WIN_NATIVE
	// WIN32 Uses TSL
	std::tm *res=::gmtime(&pt);
	if(!res)
		throw runtime_error("booster::universal_time: Failed to convert time to universal time");
	return *res;
	#else
	std::tm tm;
	if(!::gmtime_r(&pt,&tm)) {
		throw runtime_error("booster::universal_time: Failed to convert time to universal time");
	}
	return tm;
	#endif
}
time_t make_local_time(std::tm const &t)
{
	std::tm tmp = t;
	return ::mktime(&tmp);
}

time_t normalize_local_time(std::tm &t)
{
	return ::mktime(&t);
}



namespace {
	int is_leap(int year)
	{
		if(year % 400 == 0)
			return 1;
		if(year % 100 == 0)
			return 0;
		if(year % 4 == 0)
			return 1;
		return 0;
	}

	inline int days_from_0(int year)
	{
		year--;
		return 365 * year + (year / 400) - (year/100) + (year / 4);
	}

	int days_from_1970(int year)
	{
		static const int days_from_0_to_1970 = days_from_0(1970);
		return days_from_0(year) - days_from_0_to_1970;
	}

	int days_from_1jan(int year,int month,int day)
	{
		static const int days[2][12] = {
			{ 0,31,59,90,120,151,181,212,243,273,304,334 },
			{ 0,31,60,91,121,152,182,213,244,274,305,335 }
		};
		return days[is_leap(year)][month-1] + day - 1;
	}
	
	time_t my_timegm(std::tm const *t)
	{
		int year = t->tm_year + 1900;
		int month = t->tm_mon;
		if(month > 11) {
			year += month/12;
			month %= 12;
		}
		else if(month < 0) {
			int years_diff = (-month + 11)/12;
			year -= years_diff;
			month+=12 * years_diff;
		}
		month++;
		int day = t->tm_mday;
		int day_of_year = days_from_1jan(year,month,day);
		int days_since_epoch = days_from_1970(year) + day_of_year;
		
		time_t seconds_in_day = 3600 * 24;
		time_t result =  seconds_in_day * days_since_epoch + 3600 * t->tm_hour + 60 * t->tm_min + t->tm_sec;

		return result;
	}

} // anon



time_t make_universal_time(std::tm const &t)
{
	return my_timegm(&t);
}

time_t normalize_universal_time (std::tm &t)
{
	time_t pt = my_timegm(&t);
	t = universal_time(pt);
	return pt;
}

} // booster
