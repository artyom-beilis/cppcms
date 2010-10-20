//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/locale/date_time_facet.h>
#include <booster/locale/date_time.h>
#include <booster/locale/formatting.h>
#include "all_generator.h"

#include <booster/thread.h>
#include <unicode/calendar.h>
#include <unicode/gregocal.h>
#include <unicode/utypes.h>

#include <memory>
#include <math.h>

#include "cdata.h"
#include "uconv.h"

namespace booster {
namespace locale {
namespace impl_icu {
    
    static void check_and_throw_dt(UErrorCode &e)
    {
        if(U_FAILURE(e)) {
            date_time_error(u_errorName(e));
        }
    }
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
            throw booster::invalid_argument("Invalid date_time period type");
        }
    }


    class calendar_impl : public abstract_calendar {
    public:
        
        calendar_impl(cdata const &dat)
        {
            UErrorCode err=U_ZERO_ERROR;
            calendar_.reset(icu::Calendar::createInstance(dat.locale,err));
            check_and_throw_dt(err);
            encoding_ = dat.encoding;
        }
        calendar_impl(calendar_impl const &other)
        {
            calendar_.reset(other.calendar_->clone());
            encoding_ = other.encoding_;
        }

        calendar_impl *clone() const
        {
            return new calendar_impl(*this);
        }

        void set_value(period::period_type p,int value)
        {
            calendar_->set(to_icu(p),int32_t(value));
        }

        int get_value(period::period_type p,value_type type) const
        {
            UErrorCode err=U_ZERO_ERROR;
            int v=0;
            if(p==period::first_day_of_week) {
                guard l(lock_);
                v=calendar_->getFirstDayOfWeek(err);
            }
            else {
                UCalendarDateFields uper=to_icu(p);
                guard l(lock_);
                switch(type) {
                case absolute_minimum:
                    v=calendar_->getMinimum(uper);
                    break;
                case actual_minimum:
                    v=calendar_->getActualMinimum(uper,err);
                    break;
                case greatest_minimum:
                    v=calendar_->getGreatestMinimum(uper);
                    break;
                case current:
                    v=calendar_->get(uper,err);
                    break;
                case least_maximum:
                    v=calendar_->getLeastMaximum(uper);
                    break;
                case actual_maximum:
                    v=calendar_->getActualMaximum(uper,err);
                    break;
                case absolute_maximum:
                    v=calendar_->getMaximum(uper);
                    break;
                }
            }
            check_and_throw_dt(err);
            return v;
        }

        virtual void set_time(posix_time const &p)
        {
            double utime = p.seconds * 1000.0 + p.nanoseconds / 1000000.0;
            UErrorCode code=U_ZERO_ERROR;
            calendar_->setTime(utime,code);
            check_and_throw_dt(code);
        }
        virtual posix_time get_time() const
        {
            
            UErrorCode code=U_ZERO_ERROR;
            double rtime = 0;
            {
                guard l(lock_);
                rtime = calendar_->getTime(code);
            }
            check_and_throw_dt(code);
            rtime/=1000.0;
            double secs = floor(rtime);
            posix_time res;
            res.seconds = static_cast<int64_t>(secs);
            res.nanoseconds = static_cast<uint32_t>((rtime - secs) / 1e9);
            if(res.nanoseconds > 999999999)
                res.nanoseconds = 999999999;
            return res;
        }
        virtual void set_option(calendar_option_type opt,int v) 
        {
            throw booster::invalid_argument("There is no settable options for calendar");
        }
        virtual int get_option(calendar_option_type opt) const
        {
            if(opt==is_gregorian)
                return dynamic_cast<icu::GregorianCalendar const *>(calendar_.get())!=0;
            throw booster::invalid_argument("Invalid option");
        }
        virtual void adjust_value(period_type p,update_type u,int difference)
        {
            UErrorCode err=U_ZERO_ERROR;
            switch(u) {
            case move:
                calendar_->add(to_icu(p),difference,err);
                break;
            case roll:
                calendar_->roll(to_icu(p),difference,err);
                break;
            }
            check_and_throw_dt(err);
        }
        virtual int difference(abstract_calendar const *other_ptr,period::period_type p) const
        {
            calendar_impl const &other_cal=dynamic_cast<calendar_impl const &>(*other_ptr);
            std::auto_ptr<icu::Calendar> self(calendar_->clone()),other(other_cal.calendar_->clone());
            UErrorCode err=U_ZERO_ERROR;
            double other_time = 0;
            {
                guard l(other_cal.lock_);
                other_time = other->getTime(err);
            }
            check_and_throw_dt(err);
            int diff = 0;
            
            {
                guard l(lock_);
                diff = self->fieldDifference(other_time,to_icu(p),err);
            }
            check_and_throw_dt(err);
            return diff;
        }
        virtual void set_timezone(std::string const &tz)
        {
            icu_std_converter<char> cvt(encoding_);
            icu::UnicodeString utz=cvt.icu(tz.c_str(),tz.c_str()+tz.size());
            calendar_->adoptTimeZone(icu::TimeZone::createTimeZone(utz));
        }
        virtual std::string get_timezone() const 
        {
            icu::UnicodeString tz;
            calendar_->getTimeZone().getID(tz);
            icu_std_converter<char> cvt(encoding_);
            return cvt.std(tz);
        }
        virtual bool same(abstract_calendar const *other) const 
        {
            calendar_impl const *oc=dynamic_cast<calendar_impl const *>(other);
            if(!oc)
                return false;
            return calendar_->isEquivalentTo(*oc->calendar_);
        }

    private:
        typedef booster::unique_lock<booster::mutex> guard;
        mutable booster::mutex lock_;
        std::string encoding_;
        hold_ptr<icu::Calendar> calendar_;
    };
    
    class icu_calendar_facet : public calendar_facet  {
    public:
        icu_calendar_facet(cdata const &d,size_t refs = 0) : 
            calendar_facet(refs),
            data_(d)
        {
        }
        virtual abstract_calendar *create_calendar() const
        {
            return new calendar_impl(data_);
        }
    private:
        cdata data_;
    };
    
    std::locale create_calendar(std::locale const &in,cdata const &d)
    {
        return std::locale(in,new icu_calendar_facet(d));
    }

} // impl_icu
} // locale
} // boost



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

