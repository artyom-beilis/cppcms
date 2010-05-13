//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_FORMATTING_HPP_INCLUDED
#define CPPCMS_LOCALE_FORMATTING_HPP_INCLUDED

#include <cppcms/defs.h>
#include <cppcms/config.h>
#include <cppcms/cstdint.h>
#include <ostream>
#include <istream>
#include <string>

namespace cppcms {
    namespace locale {
        namespace flags {
            typedef enum {
                number              = 0,
                currency            = 1,
                date                = 2,
                time                = 3,
                datetime            = 4,
                strftime            = 5,

                display_flags_mask  = 31,

                currency_default    = 0 << 5,
                currency_iso        = 1 << 5,
                currency_national   = 2 << 5,

                currency_flags_mask = 3 << 5,

            } display_flags_type;

            typedef enum {
                datetime_pattern,
                time_zone_id
            } pattern_type;

            typedef enum {
                domain_id
            } value_type;

            
        } // flags


        CPPCMS_API uint64_t ext_flags(std::ios_base &);
        CPPCMS_API uint64_t ext_flags(std::ios_base &,flags::display_flags_type mask);
        CPPCMS_API void ext_setf(std::ios_base &,flags::display_flags_type flags,flags::display_flags_type mask);
        
        CPPCMS_API int ext_value(std::ios_base &,flags::value_type id);
        CPPCMS_API void ext_value(std::ios_base &,flags::value_type id,int value);
       
        template<typename CharType>
        void ext_pattern(std::ios_base &,flags::pattern_type pat,std::basic_string<CharType> const &);

        template<typename CharType>
        std::basic_string<CharType> ext_pattern(std::ios_base &,flags::pattern_type pattern);

        /// Specializations

        template<>
        CPPCMS_API void ext_pattern(std::ios_base &,flags::pattern_type pattern_id, std::string const &pattern);
        
        template<>
        CPPCMS_API std::string ext_pattern(std::ios_base &,flags::pattern_type pattern_id);

        namespace as {

            #define CPPCMS_LOCALE_AS_MANIPULATOR(name,mask)  \
            inline std::ios_base &name(std::ios_base &ios)  \
            {                                               \
                ext_setf(ios,flags::name,flags::mask);      \
                return ios;                                 \
            }

            CPPCMS_LOCALE_AS_MANIPULATOR(number,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(currency,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(date,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(time,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(datetime,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(strftime,display_flags_mask)

            CPPCMS_LOCALE_AS_MANIPULATOR(currency_default,currency_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(currency_iso,currency_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(currency_national,currency_flags_mask)
            
            namespace details {
                template<typename CharType>
                struct add_ftime {

                    std::basic_string<CharType> ftime;

                    void apply(std::basic_ios<CharType> &ios) const
                    {
                        ext_pattern(ios,flags::datetime_pattern,ftime);
                        as::strftime(ios);
                    }

                };

                template<typename CharType>
                std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,add_ftime<CharType> const &fmt)
                {
                    fmt.apply(out);
                    return out;
                }
                
                template<typename CharType>
                std::basic_istream<CharType> &operator>>(std::basic_istream<CharType> &in,add_ftime<CharType> const &fmt)
                {
                    fmt.apply(in);
                    return in;
                }

            }

            template<typename CharType>
            details::add_ftime<CharType> ftime(std::basic_string<CharType> const &format)
            {
                details::add_ftime<CharType> fmt;
                fmt.ftime=format;
                return fmt;
            }

            template<typename CharType>
            details::add_ftime<CharType> ftime(CharType const *format)
            {
                details::add_ftime<CharType> fmt;
                fmt.ftime=format;
                return fmt;
            }

            namespace details {
                struct set_timezone {
                    std::string id;
                };
                template<typename CharType>
                std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,set_timezone const &fmt)
                {
                    ext_pattern(out,flags::time_zone_id,fmt.id);
                    return out;
                }
                
                template<typename CharType>
                std::basic_istream<CharType> &operator>>(std::basic_istream<CharType> &in,set_timezone const &fmt)
                {
                    ext_pattern(in,flags::time_zone_id,fmt.id);
                    return in;
                }
            }
            
            
            inline std::ios_base &gmt(std::ios_base &ios)
            {
                ext_pattern<char>(ios,flags::time_zone_id,"GMT");
                return ios;
            }

            inline std::ios_base &local_time(std::ios_base &ios)
            {
                ext_pattern(ios,flags::time_zone_id,std::string());
                return ios;
            }

            inline details::set_timezone time_zone(char const *id) 
            {
                details::set_timezone tz;
                tz.id=id;
                return tz;
            }

            inline details::set_timezone time_zone(std::string const &id) 
            {
                details::set_timezone tz;
                tz.id=id;
                return tz;
            }


        } // as manipulators
        
        #undef CPPCMS_LOCALE_AS_MANIPULATOR



    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
