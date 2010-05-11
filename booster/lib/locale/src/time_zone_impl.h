//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/locale/time_zone.h>
#include <booster/locale/info.h>
#include <unicode/timezone.h>
#include <unicode/strenum.h>
#include "uconv.h"
#include "info_impl.h"

namespace booster{
    namespace locale {
        class time_zone_impl {

            // non-copyable
            time_zone_impl(time_zone_impl const &other);
            void operator=(time_zone_impl const &other);

        public:
            time_zone_impl(icu::TimeZone *ptr) : tz_(ptr) 
            {
            }
            time_zone_impl(std::string const &id) : tz_(icu::TimeZone::createTimeZone(id.c_str()))
            {
            }
            time_zone_impl() : tz_(icu::TimeZone::createDefault())
            {
            }
            time_zone_impl *clone() const
            {
                return new time_zone_impl(tz_->clone());
            }
            icu::TimeZone const *icu_tz() const
            {
                return tz_.get();
            }
            double offset(double time,bool is_local_time) const
            {
                ::int32_t raw=0;
                ::int32_t dst=0;
                UErrorCode err=U_ZERO_ERROR;
                tz_->getOffset(time*1000,UBool(is_local_time),raw,dst,err);
                return raw/1e3+dst/1e3;
            }
            std::string id() const
            {
                icu::UnicodeString tmp;
                tz_->getID(tmp);
                return std::string(tmp.getBuffer(),tmp.getBuffer()+tmp.length());
            }
            bool operator==(time_zone_impl const &other) const
            {
                return (*tz_==*other.tz_)!=FALSE; // Prevent MSVC warning
            }
            icu::UnicodeString name(std::locale const &loc) const
            {
                icu::UnicodeString tmp;
                info const &inf=std::use_facet<info>(loc);
                tz_->getDisplayName(inf.impl()->locale,tmp);
                return tmp;
            }
        private:
            std::auto_ptr<icu::TimeZone> tz_;
        };
    } //locale
} // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

