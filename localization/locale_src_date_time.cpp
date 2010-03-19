//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_date_time.h"
#include "locale_formatting.h"
#include <unicode/calendar.h>
#include <unicode/gregocal.h>
#include <unicode/utypes.h>
#include "locale_src_info_impl.hpp"
#include "locale_src_time_zone_impl.hpp"

namespace cppcms {
namespace locale {

/// UTILITY

#define CALENDAR(ptr) (reinterpret_cast<icu::Calendar *>((ptr)->impl_))
#define calendar_ CALENDAR(this)
#define const_calendar_ (const_cast<icu::Calendar const *>(calendar_))
#define const_calendar_of(other) (const_cast<icu::Calendar const *>(CALENDAR(&(other))))

using period::period_type;

static UCalendarDateFields to_icu(period_type f)
{
    using namespace period;

    switch(f) {
    case era: return UCAL_ERA;
    case year: return UCAL_YEAR;
    case extended_year: return UCAL_EXTENDED_YEAR;
    case month: return UCAL_MONTH;
    case day: return UCAL_DATE;
    case day_of_year: return UCAL_DAY_OF_YEAR;
    case day_of_week: return UCAL_DAY_OF_WEEK;
    case day_of_week_in_month:  return UCAL_DAY_OF_WEEK_IN_MONTH;
    case day_of_week_local: return UCAL_DOW_LOCAL;
    case hour: return UCAL_HOUR_OF_DAY;
    case hour_12: return UCAL_HOUR;
    case am_pm: return UCAL_AM_PM;
    case minute: return UCAL_MINUTE;
    case second: return UCAL_SECOND;
    case week_of_year: return UCAL_WEEK_OF_YEAR;
    case week_of_month: return UCAL_WEEK_OF_MONTH;
    default:
        throw std::invalid_argument("Invalid date_time period type");
    }
}

static void check_and_throw(UErrorCode &e)
{
    if(U_FAILURE(e)) {
        date_time_error(u_errorName(e));
    }
}

static void *create_calendar(std::locale const &loc=std::locale(),time_zone const &zone=time_zone())
{
    std::auto_ptr<icu::Calendar> cal;
    std::auto_ptr<icu::TimeZone> tz(zone.impl()->icu_tz()->clone());
    UErrorCode err=U_ZERO_ERROR;
    if(std::has_facet<info>(loc)) {
        cal.reset(icu::Calendar::createInstance(tz.release(),std::use_facet<info>(loc).impl()->locale,err));
    }
    else {
        cal.reset(icu::Calendar::createInstance(tz.release(),err));
    }
    if(!cal.get())
        throw date_time_error("Failed to create calendar");
    check_and_throw(err);
    return reinterpret_cast<void *>(cal.release());
}





/////////////////////////
// Calendar
////////////////////////

calendar::calendar(std::locale const &l,time_zone const &zone) :
    locale_(l),
    tz_(zone)
{
    impl_=create_calendar(locale_,tz_);
}

calendar::calendar(time_zone const &zone) :
    tz_(zone)
{
    impl_=create_calendar(locale_,tz_);
}


calendar::calendar(std::locale const &l) :
    locale_(l)
{
    impl_=create_calendar(locale_,tz_);
}

calendar::calendar(std::ios_base &ios) :
    locale_(ios.getloc()),
    tz_(ext_pattern<char>(ios,flags::time_zone_id))
{
    impl_=create_calendar(locale_,tz_);
}

calendar::calendar() 
{
    impl_=create_calendar(locale_,tz_);
}

calendar::~calendar()
{
    delete calendar_;
    impl_=0;
}

calendar::calendar(calendar const &other) :
    locale_(other.locale_),
    tz_(other.tz_)
{
    impl_=reinterpret_cast<void *>(const_calendar_of(other)->clone());
}

calendar const &calendar::operator = (calendar const &other) 
{
    if(this !=&other) {
        locale_ = other.locale_;
        tz_ = other.tz_;
        delete calendar_;
        impl_=0;
        impl_=reinterpret_cast<void *>(const_calendar_of(other)->clone());
    }
    return *this;
}

bool calendar::is_gregorian() const
{
    return dynamic_cast<icu::GregorianCalendar const *>(const_calendar_)!=FALSE;
}

cppcms::locale::time_zone calendar::get_time_zone() const
{
    return tz_;
}

std::locale calendar::get_locale() const
{
    return locale_;
}

int calendar::minimum(period_type f) const
{
    return const_calendar_->getMinimum(to_icu(f));
}

int calendar::greatest_minimum(period_type f) const
{
    return const_calendar_->getGreatestMinimum(to_icu(f));
}

int calendar::maximum(period_type f) const
{
    return const_calendar_->getMaximum(to_icu(f));
}

int calendar::least_maximum(period_type f) const
{
    return const_calendar_->getLeastMaximum(to_icu(f));
}

int calendar::first_day_of_week() const
{
    UErrorCode e=U_ZERO_ERROR;
    int first=static_cast<int>(const_calendar_->getFirstDayOfWeek(e));
    check_and_throw(e);
    return first;
}

bool calendar::operator==(calendar const &other) const
{
    return calendar_->isEquivalentTo(*const_calendar_of(other))!=FALSE;
}

bool calendar::operator!=(calendar const &other) const
{
    return !(*this==other);
}

//////////////////////////////////
// date_time
/////////////////

date_time::date_time()
{
    impl_=create_calendar();
}

date_time::date_time(date_time const &other)
{
    impl_=reinterpret_cast<void *>(const_calendar_of(other)->clone());
}

date_time::date_time(date_time const &other,date_time_period_set const &s)
{
    impl_=reinterpret_cast<void *>(const_calendar_of(other)->clone());
    for(unsigned i=0;i<s.size();i++)
        set(s[i].type,s[i].value);
}

date_time const &date_time::operator = (date_time const &other)
{
    if(this != &other) {
        delete calendar_;
        impl_=reinterpret_cast<void *>(const_calendar_of(other)->clone());
    }
    return *this;
}

date_time::~date_time()
{
    delete calendar_;
    impl_ = 0;
}

date_time::date_time(double time)
{
    impl_=create_calendar();
    UErrorCode e=U_ZERO_ERROR;
    calendar_->setTime(time * 1000.0,e);
    if(U_FAILURE(e)) {
        delete calendar_;
        check_and_throw(e);
    }
}

date_time::date_time(double time,calendar const &cal)
{
    impl_=reinterpret_cast<void *>(const_calendar_of(cal)->clone());
    UErrorCode e=U_ZERO_ERROR;
    calendar_->setTime(time * 1000.0,e);
    if(U_FAILURE(e)) {
        delete calendar_;
        check_and_throw(e);
    }
}

date_time::date_time(calendar const &cal)
{
    impl_=reinterpret_cast<void *>(const_calendar_of(cal)->clone());
    UErrorCode e=U_ZERO_ERROR;
    calendar_->setTime(icu::Calendar::getNow(),e);
    if(U_FAILURE(e)) {
        delete calendar_;
        check_and_throw(e);
    }
    
}



date_time::date_time(date_time_period_set const &s)
{
    impl_=create_calendar();
    try {
        for(unsigned i=0;i<s.size();i++)
            set(s[i].type,s[i].value);
    }
    catch(...) {
        delete calendar_;
        throw;
    }
}
date_time::date_time(date_time_period_set const &s,calendar const &cal)
{
    impl_=reinterpret_cast<void *>(const_calendar_of(cal)->clone());
    try {
        UErrorCode e=U_ZERO_ERROR;
        calendar_->setTime(icu::Calendar::getNow(),e);
        for(unsigned i=0;i<s.size();i++)
            set(s[i].type,s[i].value);
    }
    catch(...) {
        delete calendar_;
        throw;
    }

}

date_time const &date_time::operator=(date_time_period_set const &s)
{
    for(unsigned i=0;i<s.size();i++)
        set(s[i].type,s[i].value);
    return *this;
}

void date_time::set(period_type f,int v)
{
    calendar_->set(to_icu(f),v);
}

int date_time::get(period_type f) const
{
    UErrorCode e=U_ZERO_ERROR;
    int v=const_calendar_->get(to_icu(f),e);
    check_and_throw(e);
    return v;
}

date_time date_time::operator+(date_time_period const &v) const
{
    date_time tmp(*this);
    tmp+=v;
    return tmp;
}

date_time date_time::operator-(date_time_period const &v) const
{
    date_time tmp(*this);
    tmp-=v;
    return tmp;
}

date_time date_time::operator<<(date_time_period const &v) const
{
    date_time tmp(*this);
    tmp<<=v;
    return tmp;
}

date_time date_time::operator>>(date_time_period const &v) const
{
    date_time tmp(*this);
    tmp>>=v;
    return tmp;
}

date_time const &date_time::operator+=(date_time_period const &v) 
{
    UErrorCode e=U_ZERO_ERROR;
    calendar_->add(to_icu(v.type),v.value,e);
    check_and_throw(e);
    return *this;
}

date_time const &date_time::operator-=(date_time_period const &v) 
{
    UErrorCode e=U_ZERO_ERROR;
    calendar_->add(to_icu(v.type),-v.value,e);
    check_and_throw(e);
    return *this;
}

date_time const &date_time::operator<<=(date_time_period const &v) 
{
    UErrorCode e=U_ZERO_ERROR;
    calendar_->roll(to_icu(v.type),::int32_t(v.value),e);
    check_and_throw(e);
    return *this;
}

date_time const &date_time::operator>>=(date_time_period const &v) 
{
    UErrorCode e=U_ZERO_ERROR;
    calendar_->roll(to_icu(v.type),::int32_t(-v.value),e);
    check_and_throw(e);
    return *this;
}


date_time date_time::operator+(date_time_period_set const &v) const
{
    date_time tmp(*this);
    tmp+=v;
    return tmp;
}

date_time date_time::operator-(date_time_period_set const &v) const
{
    date_time tmp(*this);
    tmp-=v;
    return tmp;
}

date_time date_time::operator<<(date_time_period_set const &v) const
{
    date_time tmp(*this);
    tmp<<=v;
    return tmp;
}

date_time date_time::operator>>(date_time_period_set const &v) const
{
    date_time tmp(*this);
    tmp>>=v;
    return tmp;
}

date_time const &date_time::operator+=(date_time_period_set const &v) 
{
    for(unsigned i=0;i<v.size();i++) {
        *this+=v[i];
    }
    return *this;
}

date_time const &date_time::operator-=(date_time_period_set const &v) 
{
    for(unsigned i=0;i<v.size();i++) {
        *this-=v[i];
    }
    return *this;
}

date_time const &date_time::operator<<=(date_time_period_set const &v) 
{
    for(unsigned i=0;i<v.size();i++) {
        *this<<=v[i];
    }
    return *this;
}

date_time const &date_time::operator>>=(date_time_period_set const &v) 
{
    for(unsigned i=0;i<v.size();i++) {
        *this>>=v[i];
    }
    return *this;
}

double date_time::time() const
{
    UErrorCode e=U_ZERO_ERROR;
    double v=const_calendar_->getTime(e);
    check_and_throw(e);
    return v/1000.0;
}

void date_time::time(double v)
{
    UErrorCode e=U_ZERO_ERROR;
    calendar_->setTime(v*1000.0,e);
    check_and_throw(e);
}

bool date_time::operator==(date_time const &other) const
{
    UErrorCode e=U_ZERO_ERROR;
    bool v=const_calendar_->equals(*CALENDAR(&other),e)!=FALSE;
    check_and_throw(e);
    return v;
}

bool date_time::operator!=(date_time const &other) const
{
    return !(*this==other);
}

bool date_time::operator<(date_time const &other) const
{
    UErrorCode e=U_ZERO_ERROR;
    bool v=const_calendar_->before(*CALENDAR(&other),e)!=FALSE;
    check_and_throw(e);
    return v;
}

bool date_time::operator>=(date_time const &other) const
{
    return !(*this<other);
}

bool date_time::operator>(date_time const &other) const
{
    UErrorCode e=U_ZERO_ERROR;
    bool v=const_calendar_->after(*CALENDAR(&other),e)!=FALSE;
    check_and_throw(e);
    return v;
}

bool date_time::operator<=(date_time const &other) const
{
    return !(*this>other);
}

void date_time::swap(date_time &other)
{
    void *tmp=impl_;
    impl_=other.impl_;
    other.impl_=tmp;
}

int date_time::difference(date_time const &other,period_type f)
{
    UErrorCode e=U_ZERO_ERROR;
    int d = calendar_->fieldDifference(1000.0*other.time(),to_icu(f),e);
    check_and_throw(e);
    return d;
}


int date_time::difference(date_time const &other,period_type f) const
{
    UErrorCode e=U_ZERO_ERROR;
    std::auto_ptr<icu::Calendar> cal(const_calendar_->clone());
    int d = cal->fieldDifference(1000.0*other.time(),to_icu(f),e);
    check_and_throw(e);
    return d;
}

int date_time::maximum(period_type f) const
{
    UErrorCode e=U_ZERO_ERROR;
    int v = const_calendar_->getActualMaximum(to_icu(f),e);
    check_and_throw(e);
    return v;
}

int date_time::minimum(period_type f) const
{
    UErrorCode e=U_ZERO_ERROR;
    int v = const_calendar_->getActualMinimum(to_icu(f),e);
    check_and_throw(e);
    return v;
}


} // locale
} // boost



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

