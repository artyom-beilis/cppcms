//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_DATE_TIME_H_INCLUDED
#define BOOSTER_LOCALE_DATE_TIME_H_INCLUDED

#include <booster/config.h>
#ifdef BOOSTER_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif

#include <booster/locale/hold_ptr.h>
#include <booster/locale/date_time_facet.h>
#include <locale>
#include <vector>
#include <booster/backtrace.h>


namespace booster {
    namespace locale {
        ///
        /// \defgroup date_time Date, Time, Timezone and Calendar manipulations 
        ///
        /// This module provides various calendar, timezone and date time services
        ///
        /// @{


        ///
        /// \brief This error is thrown in case of invalid state that occurred
        ///
        class date_time_error : public booster::runtime_error {
        public:
            ///
            /// Constructor of date_time_error class
            /// 
            date_time_error(std::string const &e) : booster::runtime_error(e) {}
        };


        ///
        /// \brief This structure provides a pair period_type and amount.
        ///
        /// Usually obtained as product of period_type and integer.
        /// For example day*3 == date_time_period(day,3)
        /// 
        struct date_time_period 
        {
            period::period_type type;   ///< The type of period, i.e. era, year, day etc.
            int value;                  ///< The value the actual number of \a periods
            ///
            /// Operator + returns copy of itself
            ///
            date_time_period operator+() const { return *this; }
            ///
            /// Operator -, switches the sign of period
            ///
            date_time_period operator-() const { return date_time_period(type,-value); }
            
            ///
            /// Constructor that creates date_time_period from period_type \a f and a value \a v -- default 1.
            ///
            date_time_period(period::period_type f=period::invalid,int v=1) : type(f), value(v) {}
        };

        namespace period {
            ///
            /// Predefined constant for January
            ///
            static const date_time_period january(month,0);
            ///
            /// Predefined constant for February
            ///
            static const date_time_period february(month,1);
            ///
            /// Predefined constant for March
            ///
            static const date_time_period march(month,2);
            ///
            /// Predefined constant for April
            ///
            static const date_time_period april(month,3);
            ///
            /// Predefined constant for May
            ///
            static const date_time_period may(month,4);
            ///
            /// Predefined constant for June
            ///
            static const date_time_period june(month,5);
            ///
            /// Predefined constant for July
            ///
            static const date_time_period july(month,6);
            ///
            /// Predefined constant for August
            ///
            static const date_time_period august(month,7);
            ///
            /// Predefined constant for September
            ///
            static const date_time_period september(month,8);
            ///
            /// Predefined constant for October 
            ///
            static const date_time_period october(month,9);
            ///
            /// Predefined constant for November
            ///
            static const date_time_period november(month,10);
            ///
            /// Predefined constant for December
            ///
            static const date_time_period december(month,11);

            ///
            /// Predefined constant for Sunday
            ///
            static const date_time_period sunday(day_of_week,1);
            ///
            /// Predefined constant for Monday 
            ///
            static const date_time_period monday(day_of_week,2);
            ///
            /// Predefined constant for Tuesday
            ///
            static const date_time_period tuesday(day_of_week,3);
            ///
            /// Predefined constant for Wednesday
            ///
            static const date_time_period wednesday(day_of_week,4);
            ///
            /// Predefined constant for Thursday
            ///
            static const date_time_period thursday(day_of_week,5);
            ///
            /// Predefined constant for Friday
            ///
            static const date_time_period friday(day_of_week,6);
            ///
            /// Predefined constant for Saturday
            ///
            static const date_time_period saturday(day_of_week,7);
            ///
            /// Predefined constant for AM (Ante Meridiem)
            ///
            static const date_time_period am(am_pm,0);
            ///
            /// Predefined constant for PM (Post Meridiem)
            ///
            static const date_time_period pm(am_pm,1);

            ///
            /// convers period_type to date_time_period(f,1)
            ///
            inline date_time_period operator+(period::period_type f) 
            {
                return date_time_period(f);
            }
            ///
            /// convers period_type to date_time_period(f,-1)
            ///
            inline date_time_period operator-(period::period_type f)
            {
                return date_time_period(f,-1);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(period::period_type f,char v)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(char v,period::period_type f)
            {
                return date_time_period(f,v);
            }


            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(char v,date_time_period f)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(date_time_period f,char v)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(period::period_type f,short int v)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(short int v,period::period_type f)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(short int v,date_time_period f)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(date_time_period f,short int v)
            {
                return date_time_period(f.type,f.value*v);
            }


            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(period::period_type f,int v)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(int v,period::period_type f)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(int v,date_time_period f)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(date_time_period f,int v)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(period::period_type f,long int v)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(long int v,period::period_type f)
            {
                return date_time_period(f,v);
            }
            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(long int v,date_time_period f)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(date_time_period f,long int v)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(period::period_type f,unsigned char v)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(unsigned char v,period::period_type f)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(unsigned char v,date_time_period f)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(date_time_period f,unsigned char v)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(period::period_type f,unsigned short int v)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(unsigned short int v,period::period_type f)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(unsigned short int v,date_time_period f)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(date_time_period f,unsigned short int v)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(period::period_type f,unsigned int v)
            {
                return date_time_period(f,v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(unsigned int v,period::period_type f)
            {
                return date_time_period(f,v);
            }
            
            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(unsigned int v,date_time_period f)
            {
                return date_time_period(f.type,f.value*v);
            }
            
            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(date_time_period f,unsigned int v)
            {
                return date_time_period(f.type,f.value*v);
            }

            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(period::period_type f,unsigned long int v)
            {
                return date_time_period(f,v);
            }
            
            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(unsigned long int v,period::period_type f)
            {
                return date_time_period(f,v);
            }
            
            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(unsigned long int v,date_time_period f)
            {
                return date_time_period(f.type,f.value*v);
            }
            
            ///
            /// Create date_time_period of type \a f with value \a v. 
            ///
            inline date_time_period operator*(date_time_period f,unsigned long int v)
            {
                return date_time_period(f.type,f.value*v);
            }

        } // period


        ///
        /// \brief this class that represents a set of periods, 
        ///
        /// It is generally created by operations on periods:
        /// 1995*year + 3*month + 1*day. Note: operations are not commutative.
        ///
        class date_time_period_set {
        public:
            
            ///
            /// Default constructor - empty set
            ///
            date_time_period_set()
            {
            }
            ///
            /// Create a set of single period with value 1
            ///
            date_time_period_set(period::period_type f)
            {
                basic_[0]=date_time_period(f);
            }
            ///
            /// Create a set of single period \a fl
            ///
            date_time_period_set(date_time_period const &fl)
            {
                basic_[0]=fl;
            }
            ///
            /// Append date_time_period \a f to the set
            ///
            void add(date_time_period f)
            {
                size_t n=size();
                if(n < 4)
                    basic_[n]=f;
                else
                    periods_.push_back(f);
            }
            ///
            /// Get number if items in list
            ///
            size_t size() const
            {
                if(basic_[0].type == period::invalid)
                    return 0;
                if(basic_[1].type == period::invalid)
                    return 1;
                if(basic_[2].type == period::invalid)
                    return 2;
                if(basic_[3].type == period::invalid)
                    return 3;
                return 4+periods_.size();
            }
            ///
            /// Get item at position \a n the set, n should be in range [0,size)
            ///
            date_time_period const &operator[](unsigned n) const 
            {
                if(n >= size())
                    throw booster::out_of_range("Invalid index to date_time_period");
                if(n < 4)
                    return basic_[n];
                else
                    return periods_[n-4];
            }
        private:
            date_time_period basic_[4];
            std::vector<date_time_period> periods_;
        };

        
        ///
        /// Append two periods sets. Note this operator is not commutative 
        ///
        inline date_time_period_set operator+(date_time_period_set const &a,date_time_period_set const &b)
        {
            date_time_period_set s(a);
            for(unsigned i=0;i<b.size();i++)
                s.add(b[i]);
            return s;
        }
        
        ///
        /// Append two period sets when all periods of set \b change their sign
        ///
        inline date_time_period_set operator-(date_time_period_set const &a,date_time_period_set const &b)
        {
            date_time_period_set s(a);
            for(unsigned i=0;i<b.size();i++)
                s.add(-b[i]);
            return s;
        }


        ///
        /// \brief this class provides an access to general calendar information. 
        ///
        /// This information is not connected to specific date but generic to locale, and timezone.
        /// It is used in obtaining general information about calendar and is essential for creation of
        /// date_time objects.
        ///
        class BOOSTER_API calendar {
        public:

            ///
            /// Create calendar taking locale and timezone information from ios_base instance.
            /// 
            calendar(std::ios_base &ios);
            ///
            /// Create calendar with locale \a l and time_zone \a zone
            ///
            calendar(std::locale const &l,std::string const &zone);
            ///
            /// Create calendar with locale \a l and default timezone
            ///
            calendar(std::locale const &l);
            ///
            /// Create calendar with default locale and timezone \a zone
            ///
            calendar(std::string const &zone);
            ///
            /// Create calendar with default locale and timezone 
            ///
            calendar();
            ~calendar();

            ///
            /// copy calendar
            ///
            calendar(calendar const &other);
            ///
            /// assign calendar
            ///
            calendar const &operator=(calendar const &other);

            ///
            /// Get minimum value for period f, For example for period::day it is 1.
            ///
            int minimum(period::period_type f) const;
            ///
            /// Get grates possible minimum value for period f, For example for period::day it is 1, but may be different for other calendars.
            ///
            int greatest_minimum(period::period_type f) const;
            ///
            /// Get maximum value for period f, For example for Gregorian calendar's maximum period::day it is 31.
            ///
            int maximum(period::period_type f) const;
            ///
            /// Get least maximum value for period f, For example for Gregorian calendar's maximum period::day it is 28.
            ///
            int least_maximum(period::period_type f) const;

            ///
            /// Get first day of week for specific calendar, for example for US it is 1 - Sunday for France it is 2 - Monday
            int first_day_of_week() const;

            ///
            /// get calendar's locale
            ///
            std::locale get_locale() const;
            ///
            /// get calendar's time zone
            ///
            std::string get_time_zone() const;

            ///
            /// Check if the calendar is Gregorian
            ///
            bool is_gregorian() const;

            ///
            /// Compare calendars for equivalence: i.e. calendar types, time zones etc.
            ///
            bool operator==(calendar const &other) const;
            ///
            /// Opposite of ==
            ///
            bool operator!=(calendar const &other) const;

        private:
            friend class date_time;
            std::locale locale_;
            std::string tz_;
            hold_ptr<abstract_calendar> impl_;
        };

        ///
        /// \brief this class represents a date time and allows to perform various operation according to the
        /// locale settings.
        ///
        /// This class allows to manipulate various aspects of dates and times easily using arithmetic operations with
        /// periods.
        ///
        /// General arithmetic functions:
        ///
        /// - date_time + date_time_period_set = date_time: move time point forward by specific periods like date_time + month;
        /// - date_time - date_time_period_set = date_time: move time point backward by specific periods like date_time - month;
        /// - date_time << date_time_period_set  = date_time: roll time point forward by specific periods with rolling to begin if overflows: like "2010-01-31" << 2* day == "2010-01-02" instead of "2010-02-02"
        /// - date_time >> date_time_period_set  = date_time: roll time point backward by specific periods with rolling to end if overflows: like "2010-01-02" >> 2* day == "2010-01-31" instead of "2009-12-30"
        /// - date_time / period_type = int - current period value: like "2010-12-21" / month == 12. "2010-12-21" / year = 2010
        /// - (date_time - date_time) / period_type = int: distance between dates in period_type. Like ("2010-12-01" - "2008-12-01") / month = 24.
        ///
        /// You can also assign specific periods using assignment operator like:
        /// some_time = year * 1995 that sets the year to 1995.
        ///
        ///
        
        class BOOSTER_API date_time {
        public:

            ///
            /// Dafault constructor, uses default calendar initialized date_time object to current time.
            ///
            date_time();
            ///
            /// copy date_time
            ///
            date_time(date_time const &other);
            ///
            /// copy date_time and change some fields according to the \a set
            ///
            date_time(date_time const &other,date_time_period_set const &set);
            ///
            /// assign the date_time
            ///
            date_time const &operator=(date_time const &other);
            ~date_time();

            ///
            /// Create a date_time opject using POSIX time \a time and default calendar
            ///
            date_time(double time);
            ///
            /// Create a date_time opject using POSIX time \a time and calendar \a cal
            ///
            date_time(double time,calendar const &cal);
            ///
            /// Create a date_time opject using calendar \a cal and initializes it to current time.
            ///
            date_time(calendar const &cal);
            
            ///
            /// Create a date_time opject using default calendar and define values given in \a set
            ///
            date_time(date_time_period_set const &set);
            ///
            /// Create a date_time opject using calendar \a cal and define values given in \a set
            ///
            date_time(date_time_period_set const &set,calendar const &cal);

           
            ///
            /// assign values to valrious periods in set \a f  
            ///
            date_time const &operator=(date_time_period_set const &f);

            ///
            /// set specific period \a f value to \a v
            ///
            void set(period::period_type f,int v);
            ///
            /// get specific period \a f value
            ///
            int get(period::period_type f) const;

            ///
            /// syntactic sugar for get(f)
            ///
            int operator/(period::period_type f) const
            {
                return get(f);
            }

            ///
            /// add single period f to the current date_time
            ///
            date_time operator+(period::period_type f) const
            {
                return *this+date_time_period(f);
            }

            ///
            /// subtract single period f from the current date_time
            ///
            date_time operator-(period::period_type f) const
            {
                return *this-date_time_period(f);
            }

            ///
            /// add single period f to the current date_time
            ///
            date_time const &operator+=(period::period_type f)
            {
                return *this+=date_time_period(f);
            }
            ///
            /// subtract single period f from the current date_time
            ///
            date_time const &operator-=(period::period_type f)
            {
                return *this-=date_time_period(f);
            }

            ///
            /// roll forward a date by single period f.
            ///
            date_time operator<<(period::period_type f) const
            {
                return *this<<date_time_period(f);
            }

            ///
            /// roll backward a date by single period f.
            ///
            date_time operator>>(period::period_type f) const
            {
                return *this>>date_time_period(f);
            }

            ///
            /// roll forward a date by single period f.
            ///
            date_time const &operator<<=(period::period_type f)
            {
                return *this<<=date_time_period(f);
            }
            ///
            /// roll backward a date by single period f.
            ///
            date_time const &operator>>=(period::period_type f)
            {
                return *this>>=date_time_period(f);
            }

            ///
            /// add date_time_period to the current date_time
            ///
            date_time operator+(date_time_period const &v) const;
            ///
            /// substract date_time_period from the current date_time
            ///
            date_time operator-(date_time_period const &v) const;
            ///
            /// add date_time_period to the current date_time
            ///
            date_time const &operator+=(date_time_period const &v);
            ///
            /// substract date_time_period from the current date_time
            ///
            date_time const &operator-=(date_time_period const &v);

            ///
            /// roll current date_time forward by date_time_period v
            ///
            date_time operator<<(date_time_period const &v) const;
            ///
            /// roll current date_time backward by date_time_period v
            ///
            date_time operator>>(date_time_period const &v) const ;
            ///
            /// roll current date_time forward by date_time_period v
            ///
            date_time const &operator<<=(date_time_period const &v);
            ///
            /// roll current date_time backward by date_time_period v
            ///
            date_time const &operator>>=(date_time_period const &v);

            ///
            /// add date_time_period_set v to the current date_time
            ///
            date_time operator+(date_time_period_set const &v) const;
            ///
            /// substract date_time_period_set v from the current date_time
            ///
            date_time operator-(date_time_period_set const &v) const;
            ///
            /// add date_time_period_set v to the current date_time
            ///
            date_time const &operator+=(date_time_period_set const &v);
            ///
            /// substract date_time_period_set v from the current date_time
            ///
            date_time const &operator-=(date_time_period_set const &v);

            ///
            /// roll current date_time forward by date_time_period_set v
            ///
            date_time operator<<(date_time_period_set const &v) const;
            ///
            /// roll current date_time backward by date_time_period_set v
            ///
            date_time operator>>(date_time_period_set const &v) const ;
            ///
            /// roll current date_time forward by date_time_period_set v
            ///
            date_time const &operator<<=(date_time_period_set const &v);
            ///
            /// roll current date_time backward by date_time_period_set v
            ///
            date_time const &operator>>=(date_time_period_set const &v);

            ///
            /// Get POSIX time
            ///
            /// The POSIX time is number of seconds since January 1st, 1970 00:00 UTC, ignoring leap seconds.
            ///
            double time() const;
            ///
            /// set POSIX time
            ///
            /// The POSIX time is number of seconds since January 1st, 1970 00:00 UTC, ignoring leap seconds.
            /// This time can be fetched from Operating system clock using C function time, gettimeofdat and others.
            ///
            void time(double v);

            ///
            /// compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator==(date_time const &other) const;
            ///
            /// compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator!=(date_time const &other) const;
            ///
            /// compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator<(date_time const &other) const;
            ///
            /// compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator>(date_time const &other) const;
            ///
            /// compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator<=(date_time const &other) const;
            ///
            /// compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator>=(date_time const &other) const;

            ///
            /// swaps two dates - efficient, does not throw
            ///
            void swap(date_time &other);

            ///
            /// calculate the distance from this date_time to \a other in terms of perios \a f
            ///
            int difference(date_time const &other,period::period_type f) const;

            ///
            /// Get minimal possible value for current time point for a period \a f.
            ///
            int minimum(period::period_type f) const;
            ///
            /// Get minimal possible value for current time point for a period \a f. For example
            /// in February maximum(day) may be 28 or 29, in January maximum(day)==31
            ///
            int maximum(period::period_type f) const;

        private:
            hold_ptr<abstract_calendar> impl_;
        };

        ///
        /// Writes date_time \a t to output stream \a out.
        ///
        /// This function uses locale, calendar and time zone of the target stream \a in.
        ///
        /// For example:
        /// \code
        ///  date_time now(time(0),hebrew_calendar)
        ///  cout << "Year:" t / perood::year <<" Full Date:"<< as::date_time << t;
        /// \endcode
        ///
        /// The output may be Year:5770 Full Date:Jan 1, 2010
        /// 
        template<typename CharType>
        std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,date_time const &t)
        {
            out << t.time();
            return out;
        }

        ///
        /// Reads date_time \a t from output stream \a in
        ///
        /// This function uses locale, calendar and time zone of the source stream \a in.
        ///
        template<typename CharType>
        std::basic_istream<CharType> &operator>>(std::basic_istream<CharType> &in,date_time &t)
        {
            double v;
            in >> v;
            t.time(v);
            return in;
        }

        ///
        /// \brief This class represents a period: a pair of two date_time objects.
        /// 
        /// It is generally used as syntactic sugar to calculate difference between two dates.
        ///
        /// Note: it stores references to the original objects, so it is not recommended to be used
        /// outside of the equation you calculate the difference in.
        ///
        class date_time_duration {
        public:

            ///
            /// Create an object were \a first represents earlier point on time line and \a second is later
            /// point.
            /// 
            date_time_duration(date_time const &first,date_time const &second) :
                s_(first),
                e_(second)
            {
            }

            ///
            /// find a difference in terms of period_type \a f
            ///
            int operator / (period::period_type f) const
            {
                return start().difference(end(),f);
            }

            ///
            /// Get starting point
            ///
            date_time const &start() const { return s_; }
            ///
            /// Get ending point
            ///
            date_time const &end() const { return e_; }
        private:
            date_time const &s_;
            date_time const &e_;
        };

        ///
        /// Calculates the difference between two dates, the left operand is a later point on time line.
        /// Returns date_time_duration object.
        ///
        inline date_time_duration operator-(date_time const &later,date_time const &earlier)
        {
            return date_time_duration(earlier,later);
        }


        ///
        /// \brief namespace that holds function for operating global time zone identifier
        ///
        namespace time_zone {
            ///
            /// Get global time zone identifier. If empty, system time zone is used
            ///
            BOOSTER_API std::string global();
            ///
            /// Set global time zone identifier returing pervious one. If empty, system time zone is used
            ///
            BOOSTER_API std::string global(std::string const &new_tz);
        }

        /// @}



    } // locale
} // boost

#ifdef BOOSTER_MSVC
#pragma warning(pop)
#endif


#endif
///
/// \example calendar.cpp
///
/// Example of using date_time functions for generating calendar for current year.
///

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
