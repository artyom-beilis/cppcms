//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <booster/locale/date_time.h>
#include <booster/locale/generator.h>
#include <booster/locale/formatting.h>
#include <iomanip>
#include "test_locale.h"

#ifdef BOOSTER_MSVC
#  pragma warning(disable : 4244) // loose data 
#endif

#define RESET() do { time_point = base_time_point; ss.str(""); } while(0)
#define TESTR(X) do { TEST(X); RESET(); } while(0)
//#define TESTEQSR(t,X) do { ss << (t); TESTR(ss.str() == X); } while(0)
#define TESTEQSR(t,X) do { ss << (t); if(ss.str()!=X) { std::cerr <<"[" << ss.str() <<"]" << std::endl; } TESTR(ss.str() == X); } while(0)

int main()
{
    try {
        using namespace booster::locale;
        using namespace booster::locale::period;
        
        booster::locale::generator g;

        std::locale loc=g("en_US.UTF-8");

        std::locale::global(loc);
        
        std::string tz("GMT");
        time_zone::global(tz);
        calendar cal(loc,tz); 

        TEST(calendar() == cal);
        TEST(calendar(loc) == cal);
        TEST(calendar(tz) == cal);
        TEST(calendar(loc,"GMT+01:00") != cal);
        TEST(calendar(g("ru_RU.UTF-8")) != cal);

        TEST(cal.minimum(month)==0);
        TEST(cal.maximum(month)==11);
        TEST(cal.minimum(day)==1);
        TEST(cal.greatest_minimum(day)==1);
        TEST(cal.least_maximum(day)==28);
        TEST(cal.maximum(day)==31);

        TEST(calendar(g("ar_EG.UTF-8")).first_day_of_week() == 7);
        TEST(calendar(g("he_IL.UTF-8")).first_day_of_week() == 1);
        TEST(calendar(g("ru_RU.UTF-8")).first_day_of_week() == 2);

        std::ostringstream ss;
        ss.imbue(loc);
        ss<<booster::locale::as::time_zone(tz);
        
        date_time time_point;
        
        time_point=year * 1970 + february + 5 * day;

        ss << as::date << time_point;

        TEST(ss.str() == "Feb 5, 1970");
        time_point = 3 * hour_12 + 1 * am_pm + 33 * minute + 13 * second; 
        ss.str("");
        ss << as::datetime << time_point;
        std::cerr << ss.str() << std::endl;
        TEST( ss.str() == "Feb 5, 1970 3:33:13 PM"); ss.str("");

        time_t a_date = 3600*24*(31+4); // Feb 5th
        time_t a_time = 3600*15+60*33; // 15:33:05
        time_t a_timesec = 13;
        time_t a_datetime = a_date + a_time + a_timesec;

        date_time base_time_point=date_time(a_datetime);

        RESET();

        time_point += hour;
        TESTEQSR(time_point,"Feb 5, 1970 4:33:13 PM");

        TEST(time_point.minimum(day)==1);
        TEST(time_point.maximum(day)==28);

        time_point += year * 2 + 1 *month;
        TESTEQSR(time_point,"Mar 5, 1972 3:33:13 PM");

        time_point -= minute;
        TESTEQSR( time_point, "Feb 5, 1970 3:32:13 PM");

        time_point <<= minute * 30;
        TESTEQSR( time_point, "Feb 5, 1970 3:03:13 PM"); 

        time_point >>= minute * 40;
        TESTEQSR( time_point, "Feb 5, 1970 3:53:13 PM");

        TEST((time_point + month) / month == 2);
        TEST(time_point / month == 1);
        TEST((time_point - month) / month == 0);
        TEST(time_point / month == 1);
        TEST((time_point << month) / month == 2);
        TEST(time_point / month == 1);
        TEST((time_point >> month) / month == 0);
        TEST(time_point / month == 1);



        TEST( (time_point + 2 * hour - time_point) / minute == 120);
        TEST( (time_point + month - time_point) / day == 28);
        TEST( (time_point + 2* month - (time_point+month)) / day == 31);

        TESTEQSR( time_point + hour, "Feb 5, 1970 4:33:13 PM"); 
        TESTEQSR( time_point - 2 * hour, "Feb 5, 1970 1:33:13 PM");
        TESTEQSR( time_point >> minute, "Feb 5, 1970 3:32:13 PM"); 
        TESTEQSR( time_point << second, "Feb 5, 1970 3:33:14 PM");

        TEST(time_point == time_point);
        TEST(!(time_point != time_point));
        TEST(time_point.get(hour) == 15);
        TEST(time_point/hour == 15);
        TEST(time_point+year != time_point);
        TEST(time_point - minute <= time_point);
        TEST(time_point <= time_point);
        TEST(time_point + minute >= time_point);
        TEST(time_point >= time_point);

        TEST(time_point < time_point + second);
        TEST(!(time_point < time_point - second));
        TEST(time_point > time_point - second);
        TEST(!(time_point > time_point + second));

        TEST(time_point.get(day) == 5);
        TEST(time_point.get(year) == 1970);

        TEST(time_point.get(era) == 1);
        TEST(time_point.get(year) == 1970);
        TEST(time_point.get(extended_year) == 1970);
        time_point=extended_year * -3;
        TEST(time_point.get(era) == 0);
        TEST(time_point.get(year) == 4);
        RESET();
        TEST(time_point.get(month) == 1);
        TEST(time_point.get(day) == 5);
        TEST(time_point.get(day_of_year) == 36);
        TEST(time_point.get(day_of_week) == 5);
        // TODO
        // TEST(time_point.get(day_of_week_in_month)==?);
        //
        time_point=date_time(a_datetime,calendar(g("ru_RU.UTF-8")));
        TEST(time_point.get(day_of_week_local) == 4);
        RESET();
        TEST(time_point.get(hour) == 15);
        TEST(date_time(a_datetime,calendar("GMT+01:00")).get(hour) ==16);
        TEST(time_point.get(hour_12) == 3);
        TEST(time_point.get(am_pm) == 1);
        TEST(time_point.get(minute) == 33);
        TEST(time_point.get(second) == 13);
        TEST(date_time(year* 1984 + february + day).get(week_of_year)==5);
        TEST(time_point.get(week_of_month) == 1);

    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
