//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_TIMEZONE_HPP_INCLUDED
#define CPPCMS_LOCALE_TIMEZONE_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#include <string>
#include <set>
#include <ostream>
#include <memory>

namespace cppcms {
    namespace locale {

        class time_zone_impl;

        ///
        /// \brief class used for calculation of time zone offsets
        ///
        class time_zone {
        public:
            
            ///
            /// Create GMT Time Zone
            ///
            time_zone();

            time_zone(time_zone const &other);
            time_zone const &operator=(time_zone const &other);
            ~time_zone();
            ///
            /// Create time zone with id \a id
            ///
            time_zone(std::string const &id);

            bool operator==(time_zone const &other) const;

            bool operator!=(time_zone const &other) const
            {
                return !(*this==other);
            }


            ///
            /// Get time zone id
            /// 
            std::string id() const;

            ///
            /// Find an offset in seconds from GMT for time \a time, If \a time represents locale time \a is_locale_time
            /// should be set to true
            /// 
            double offset_from_gmt(double time,bool is_local_time = false) const;

            ///
            /// Adjust locale time to GMT time
            ///
            template<typename TimeType>
            TimeType to_gmt(TimeType local_time) const
            {
                return local_time - offset_from_gmt(static_cast<double>(local_time),true);
            }

            ///
            /// Adjust GMT time to local time
            ///
            template<typename TimeType>
            TimeType to_local(TimeType gmt_time) const
            {
                return gmt_time + offset_from_gmt(static_cast<double>(gmt_time),false);
            }
            
            ///
            /// Get a list of all supported time zone ids
            ///
            static std::set<std::string> all_zones();

            ///
            /// For internal Use only -- don't use it
            ///

            time_zone_impl *impl() const
            {
                return impl_.get();
            }
        private:
            std::auto_ptr<time_zone_impl> impl_;
        };

        ///
        /// Write time zone in human readable format to stream. Note this is not the same as switching time zone of the
        /// stream. If you want to switch time zone use manipulator as::time_zone
        ///
        template<typename CharType>
        std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,time_zone const &tz);
        
        template<>
        CPPCMS_API std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out,time_zone const &tz);
    
        #ifndef CPPCMS_NO_STD_WSTRING
        template<>
        CPPCMS_API std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &out,time_zone const &tz);
        #endif
    
        
        #ifdef CPPCMS_HAS_CHAR16_T
        template<>
        CPPCMS_API std::basic_ostream<char16_t> &operator<<(std::basic_ostream<char16_t> &out,time_zone const &tz);
        #endif
    
        #ifdef CPPCMS_HAS_CHAR32_T
        template<>
        CPPCMS_API std::basic_ostream<char32_t> &operator<<(std::basic_ostream<char32_t> &out,time_zone const &tz);
        #endif
    
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
