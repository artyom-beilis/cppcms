//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <locale>
#include <string>
#include <ios>
#include <booster/locale/date_time_facet.h>
#include <sstream>
#include <stdlib.h>

#include "timezone.h"

namespace booster {
namespace locale {
namespace util {

    namespace {

        bool comparator(char const *left,char const *right)
        {
            return strcmp(left,right) < 0;
        }
        
        //
        // Ref: CLDR 1.9 common/supplemental/supplementalData.xml
        //
        // monday - default
        // fri - MV
        // sat - AE AF BH DJ DZ EG ER ET IQ IR JO KE KW LY MA OM QA SA SD SO SY TN YE
        // sun - AR AS AZ BW CA CN FO GE GL GU HK IL IN JM JP KG KR LA MH MN MO MP MT NZ PH PK SG TH TT TW UM US UZ VI ZW
        //

        int first_day_of_week(char const *terr) {
            static char const * const sat[] = {
                "AE","AF","BH","DJ","DZ","EG","ER","ET","IQ","IR",
                "JO","KE","KW","LY","MA","OM","QA","SA","SD","SO",
                "SY","TN","YE"
            };
            static char const * const sun[] = {
                "AR","AS","AZ","BW","CA","CN","FO","GE","GL","GU",
                "HK","IL","IN","JM","JP","KG","KR","LA","MH","MN",
                "MO","MP","MT","NZ","PH","PK","SG","TH","TT","TW",
                "UM","US","UZ","VI","ZW" 
            };
            if(strcmp(terr,"MV") == 0)
                return 5; // fri
            if(std::binary_search<char const * const *>(sat,sat+sizeof(sat)/(sizeof(sat[0])),terr,comparator))
                return 6; // sat
            if(std::binary_search<char const * const *>(sun,sun+sizeof(sun)/(sizeof(sun[0])),terr,comparator))
                return 0; // sun
            // default
            return 1; // mon
        }
    }


    class gregorian_calendar : public abstract_calendar {
    public:
            
            gregorian_calendar(std::string const &terr)
            {
                first_day_of_week_ = first_day_of_week(terr.c_str());
                time_ = time(0);
                is_local_ = true;
                tzoff_ = 0;
                from_time(time_);
            }
                
            ///
            /// Make a polymorphic copy of the calendar
            ///
            virtual abstract_calendar *clone() const
            {
                return new gregorian_calendar(*this);
            }

            ///
            /// Set specific \a value for period \a p, note not all values are settable.
            ///
            virtual void set_value(period::period_type p,int value) 
            {
                std::tm t;
                switch(p) {
                case era:                        ///< Era i.e. AC, BC in Gregorian and Julian calendar, range [0,1]
                    return;
                case year:                       ///< Year, it is calendar specific
                case extended_year:              ///< Extended year for Gregorian/Julian calendars, where 1 BC == 0, 2 BC == -1.
                    t.tm_year = value - 1900;
                    break;
                case month:
                    t.tm_mon = value;
                    break;
                case day:
                    t.tm_mday = value;
                    break;
                case hour:                       ///< 24 clock hour [0..23]
                    t.tm_hour = value;
                    break;
                case hour_12:                    ///< 12 clock hour [0..11]
                    t.tm_hour = t.tm_hour / 12 * 12 + value;
                    break;
                case am_pm:                      ///< am or pm marker, [0..1]
                    t.tm_hour = 12 * value + t.tm_hour % 12;
                    break;
                case minute:                     ///< minute [0..59]
                    t.tm_min = value;
                    break;
                case second:
                    t.tm_sec = value;
                    break;
                case day_of_week:                ///< Day of week, starting from Sunday, [1..7]
                case day_of_week_local: ///< Local day of week, for example in France Monday is 1, in US Sunday is 1, [1..7]
                    // FIXME
                    break;
                case day_of_year:
                    t.tm_mday += (day_of_year - t.tm_yday);
                    break;
                case day_of_week_in_month:       ///< Original number of the day of the week in month.
                case week_of_year:               ///< The week number in the year
                case week_of_month:              ///< The week number withing current month
                    // FIXME
                    break;
                case first_day_of_week:          ///< For example Sunday in US, Monday in France
                    return;
                }
                from_tm(t);
            }

            ///
            /// Get specific value for period \a p according to a value_type \a v
            ///
            virtual int get_value(period::period_type p,value_type v) const 
            {
                switch(p) {
                case era;
                    return 1;
                case year:
                case extended_year:
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        #ifdef BOOSTER_WIN_NATIVE
                        return 1970; // Unix epoch windows can't handle negative time_t
                        #else
                        if(sizeof(time_t) == 4)
                            return 1901; // minimal year with 32 bit time_t
                        else
                            return 1; 
                        #endif
                    case absolute_maximum:
                    case least_maximum:
                    case actual_maximum:
                        if(sizeof(time_t) == 4)
                            return 2038; // Y2K38 - maximal with 32 bit time_t
                        else
                            return std::numeric_limits<int>::max();
                    case current:
                        return tm_.tm_year + 1900;
                    };
                    break;
                case month:
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 0;
                    case absolute_maximum:
                    case least_maximum:
                    case actual_maximum:
                        return 11;
                    case current:
                        return tm_.tm_mon;
                    };
                    break;
                case day:
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 1;
                    case absolute_maximum:
                        return 31;
                    case least_maximum:
                        return 28;
                    case actual_maximum:
                        switch(tm_.tm_mon + 1){
                            case 1:
                            case 3:
                            case 5:
                            case 7:
                            case 8:
                            case 10:
                            case 12:
                                return 31;
                            case 4:
                            case 6:
                            case 9:
                            case 11:
                                return 30;
                            case 2:
                                return is_leap() ? 29 : 28;
                        }
                        break;
                    case current:
                        return tm_.tm_mday;
                    };
                    break;
                case day_of_year:                ///< The number of day in year, starting from 1
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 1;
                    case absolute_maximum:
                        return 366;
                    case least_maximum:
                        return 365;
                    case actual_maximum:
                        return is_leap() ? 366 : 365;
                    case current:
                        return tm_.tm_yday;
                    }
                    break;
                case day_of_week:                ///< Day of week, starting from Sunday, [1..7]
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 1;
                    case absolute_maximum:
                    case least_maximum:
                    case actual_maximum:
                        return 7;
                    case current:
                        return tm_.tm_wday + 1;
                    }
                    break;
                case day_of_week_local:     ///< Local day of week, for example in France Monday is 1, in US Sunday is 1, [1..7]
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 1;
                    case absolute_maximum:
                    case least_maximum:
                    case actual_maximum:
                        return 7;
                    case current:
                        return (tm_.tm_wday - first_day_of_week_) % 7 + 1;
                    }
                    break;
                case hour:                       ///< 24 clock hour [0..23]
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 0;
                    case absolute_maximum:
                    case least_maximum:
                    case actual_maximum:
                        return 23;
                    case current:
                        return tm_.tm_hour;
                    }
                    break;
                case hour_12:                    ///< 12 clock hour [0..11]
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 0;
                    case absolute_maximum:
                    case least_maximum:
                    case actual_maximum:
                        return 11;
                    case current:
                        return tm_.tm_hour % 12;
                    }
                    break;
                case am_pm:                      ///< am or pm marker, [0..1]
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 0;
                    case absolute_maximum:
                    case least_maximum:
                    case actual_maximum:
                        return 1;
                    case current:
                        return tm_.tm_hour >= 12 ? 1 : 0;
                    }
                    break;
                case minute:                     ///< minute [0..59]
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 0;
                    case absolute_maximum:
                    case least_maximum:
                    case actual_maximum:
                        return 59;
                    case current:
                        return tm_.tm_min;
                    }
                    break;
                case second:                     ///< second [0..59]
                    switch(v) {
                    case absolute_minimum:
                    case greatest_minimum:
                    case actual_minimum:
                        return 0;
                    case absolute_maximum:
                    case least_maximum:
                    case actual_maximum:
                        return 59;
                    case current:
                        return tm_.tm_sec;
                    }
                    break;
                case first_day_of_week:          ///< For example Sunday in US, Monday in France
                    return first_day_of_week_ + 1;
                case week_of_year:               ///< The week number in the year
                case week_of_month:              ///< The week number withing current month
                case day_of_week_in_month:       ///< Original number of the day of the week in month.
                    // FIXME
                    ;
                }
                return 0;
 
            }

            ///
            /// Set current time point
            ///
            virtual void set_time(posix_time const &p)
            {
                from_time(static_cast<time_t>(p.seconds));
            }
            virtual posix_time get_time() const  
            {
                posix_time pt = { time_, 0};
                return pt;
            }

            ///
            /// Set option for calendar, for future use
            ///
            virtual void set_option(calendar_option_type /*opt*/,int /*v*/)
            {
                // Nothing to do now
            }
            ///
            /// Get option for calendar, currently only check if it is Gregorian calendar
            ///
            virtual int get_option(calendar_option_type opt) const 
            {
                switch(opt) {
                case is_gregorian:
                    return 1;
                default:
                    return 0;
                };
            }

            ///
            /// Adjust period's \a p value by \a difference items using a update_type \a u.
            /// Note: not all values are adjustable
            ///
            virtual void adjust_value(period::period_type p,update_type u,int difference)
            {
                switch(update_type) {
                case move:
                    set_value(p,get_value(p,current) + difference);
                    break;
                case roll:
                    { // roll
                        int cur_min = get_value(p,actual_minimum);
                        int cur_max = get_value(p,actual_maximum);
                        int max_diff = cur_max - cur_min;
                        if(max_diff > 0) {
                            int value = get_value(p,current);
                            value = (value - cur_min + difference) % max_diff + cur_min;
                            set_value(p,value);
                        }
                    }
                default:
                    ;
                }
            }

            ///
            /// Calculate the difference between this calendar  and \a other in \a p units
            ///
            virtual int difference(abstract_calendar const *other_cal,period::period_type p) const 
            {
                std::auto_ptr<gregorian_calendar> other(clone());
                other->set_time(other_cal->get_time());
                switch(p) {
                case year:
                case extended_year:
                    return other->tm_.tm_year - tm_.year;
                case month:
                    return (other->tm_.tm_year - tm_.year) * 12 
                        + other->tm_.tm_mon_ - tm_->tm_mon;
                case day:
                case day_of_year:
                case day_of_week:
                case day_of_week_in_month:
                case day_of_week_local:
                case week_of_month:
                    ;
                    
                };
            }

            ///
            /// Set time zone, empty - use system
            ///
            virtual void set_timezone(std::string const &tz)
            {
                if(tz.empty()) {
                    is_local_ = true;
                    tzoff_ = 0;
                }
                else {
                    is_local_ = false;
                    tzoff_ = parse_tz(tz);
                }
                from_time(time_);
                time_zone_name_ = tz;
            }
            virtual std::string get_timezone() const
            {
                return time_zone_name_;
            }

            virtual bool same(abstract_calendar const *other) const 
            {
                gregorian_calendar const *gcal = dynamic_cast<gregorian_calendar const *>(other);
                if(!gcal)
                    return false;
                return 
                    gcal->tzoff_ == tzoff_ 
                    && gcal_->is_local_ == is_local_
                    && gcal_->first_day_of_week_  == first_day_of_week_;
            }

            virtual ~abstract_calendar()
            {
            }

    private:
        void from_tm(std::tm val)
        {
            val.tm_isdst = -1;
            val.tm_wday = -1; // indecatpr of error
            time_t point = -1;
            if(is_local_) {
                point = mktime(&val);
            }
            else {
                point = my_timegm(&val);
            }
            
            if(point == static_cast<time_t>(-1)){
                #ifndef BOOSTER_WIN_NATIVE
                // windows does not handle negative time_t, under 
                // linux it may be actually valid so we check other fields
                if(val.tm_wday == -1)
                #endif
                {
                    throw booster::invalid_argument("boost::locale::gregorian_calendar: invalid time");
                }
            }
            time_ = point - tzoff_;
        }

        void from_time(time_t point)
        {
            time_t real_point = point + tzoff_;
            std::tm *t = 0;
            #ifdef BOOSTER_WIN_NATIVE
            t = is_local_ ? localtime(&real_point) : gmtime(&real_point);
            #else
            std::tm tmp_tm;
            t = is_local_ ? localtime_r(&real_point,&tmp_tm) : gmtime_r(&real_point,&tmp_tm);
            #endif
            if(!t) {
                throw booster::invalid_argument("boost::locale::gregorian_calendar: invalid time point");
            }
            tm_ = *t;
            time_ = point;
        }
        int first_day_of_week_;
        time_t time_;
        std::tm tm_;
        bool is_local_;
        int tzoff_;
        std::string time_zone_name_;
        
    };


} // util
} // locale 
} //boost




#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
