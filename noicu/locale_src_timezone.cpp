//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_timezone.h"
#include "locale_info.h"
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/noncopyable.hpp>
#else // Internal Boost
#   include <cppcms_boost/noncopyable.hpp>
    namespace boost = cppcms_boost;
#endif
#include <unicode/timezone.h>
#include <unicode/strenum.h>
#include "locale_src_uconv.hpp"
#include "locale_src_info_impl.hpp"

namespace cppcms{
    namespace locale {
        class time_zone_impl : public boost::noncopyable {
        public:
            time_zone_impl(icu::TimeZone *ptr) : tz_(ptr) 
            {
            }
            time_zone_impl(std::string const &id) : tz_(icu::TimeZone::createTimeZone(id.c_str()))
            {
            }
            time_zone_impl() : tz_(icu::TimeZone::getGMT()->clone())
            {
            }
            time_zone_impl *clone() const
            {
                return new time_zone_impl(tz_->clone());
            }
            double offset(double time,bool is_local_time) const
            {
                int32_t raw=0;
                int32_t dst=0;
                UErrorCode err=U_ZERO_ERROR;
                tz_->getOffset(time*1000,is_local_time,raw,dst,err);
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
                return *tz_==*other.tz_;
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

        time_zone::time_zone() : impl_(new time_zone_impl())
        {

        }
        time_zone::~time_zone()
        {
        }
        time_zone::time_zone(time_zone const &other) :
            impl_(other.impl_->clone())
        {
        }
        time_zone const &time_zone::operator=(time_zone const &other) 
        {
            if(&other!=this) {
                impl_.reset(other.impl_->clone());
            }
            return *this;
        }

        time_zone::time_zone(std::string const &id) : impl_(new time_zone_impl(id))
        {
        }
        std::string time_zone::id() const
        {
            return impl_->id();
        }
        double time_zone::offset_from_gmt(double time,bool islt) const
        {
            return impl_->offset(time,islt);
        }

        bool time_zone::operator==(time_zone const &other) const
        {
            return *impl_==*other.impl_;
        }
       
        std::set<std::string> time_zone::all_zones()
        {
            std::auto_ptr<icu::StringEnumeration> all(icu::TimeZone::createEnumeration());
            std::set<std::string> result;
            UErrorCode err=U_ZERO_ERROR;
            char const *str;
            while((str=all->next(0,err))!=0) {
                err=U_ZERO_ERROR;
                result.insert(str);
            }
            return result;
        }

        namespace {
            template<typename CharType>
            void write_string(std::basic_ostream<CharType> &out,icu::UnicodeString const &str)
            {
                impl::icu_std_converter<CharType> cvt(std::use_facet<info>(out.getloc()).encoding());
                out << cvt.std(str);
            }
        }
        
        template<>
        CPPCMS_API std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out,time_zone const &tz)
        {
            write_string(out,tz.impl()->name(out.getloc()));
            return out;
        }
    
        #ifndef CPPCMS_NO_STD_WSTRING
        template<>
        CPPCMS_API std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &out,time_zone const &tz)
        {
            write_string(out,tz.impl()->name(out.getloc()));
            return out;
        }
        #endif
    
        
        #ifdef CPPCMS_HAS_CHAR16_T
        template<>
        CPPCMS_API std::basic_ostream<char16_t> &operator<<(std::basic_ostream<char16_t> &out,time_zone const &tz)
        {
            write_string(out,tz.impl()->name(out.getloc()));
            return out;
        }
        #endif
    
        #ifdef CPPCMS_HAS_CHAR32_T
        template<>
        CPPCMS_API std::basic_ostream<char32_t> &operator<<(std::basic_ostream<char32_t> &out,time_zone const &tz)
        {
            write_string(out,tz.impl()->name(out.getloc()));
            return out;
        }
        #endif

    }
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

