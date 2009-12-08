//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_FORMATTING_HPP_INCLUDED
#define CPPCMS_LOCALE_FORMATTING_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#include "locale_timezone.h"
#include "cstdint.h"
#include <ostream>
#include <istream>
#include <string>

namespace cppcms {
    namespace locale {
        namespace flags {
            typedef enum {
                posix               = 0,
                number              = 1,
                currency            = 2,
                percent             = 3,
                date                = 4,
                time                = 5,
                datetime            = 6,
                strftime            = 7,
                spellout            = 8,
                ordinal             = 9,

                display_flags_mask  = 31,

                currency_default    = 0 << 5,
                currency_iso        = 1 << 5,
                currency_national   = 2 << 5,

                currency_flags_mask = 3 << 5,

                time_default        = 0 << 7,
                time_short          = 1 << 7,
                time_medium         = 2 << 7,
                time_long           = 3 << 7,
                time_full           = 4 << 7,
                time_flags_mask     = 7 << 7,

                date_default        = 0 << 10,
                date_short          = 1 << 10,
                date_medium         = 2 << 10,
                date_long           = 3 << 10,
                date_full           = 4 << 10,
                date_flags_mask     = 7 << 10,

                datetime_flags_mask = date_flags_mask | time_flags_mask

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

        #ifndef CPPCMS_NO_STD_WSTRING
        
        template<>
        CPPCMS_API void ext_pattern(std::ios_base &,flags::pattern_type pattern_id, std::wstring const &pattern);

        template<>
        CPPCMS_API std::wstring ext_pattern(std::ios_base &,flags::pattern_type pattern_id);

        #endif // CPPCMS_NO_STD_WSTRING

        #ifdef CPPCMS_HAS_CHAR16_T
        template<>
        CPPCMS_API void ext_pattern(std::ios_base &,flags::pattern_type pattern_id, std::u16string const &pattern);

        template<>
        CPPCMS_API std::u16string ext_pattern(std::ios_base &,flags::pattern_type pattern_id);
        #endif // char16_t, u16string

        #ifdef CPPCMS_HAS_CHAR32_T
        template<>
        CPPCMS_API void ext_pattern(std::ios_base &,flags::pattern_type pattern_id, std::u32string const &pattern);

        template<>
        CPPCMS_API std::u32string ext_pattern(std::ios_base &,flags::pattern_type pattern_id);
        #endif // char32_t, u32string


        namespace as {

            #define CPPCMS_LOCALE_AS_MANIPULATOR(name,mask)  \
            inline std::ios_base &name(std::ios_base &ios)  \
            {                                               \
                ext_setf(ios,flags::name,flags::mask);      \
                return ios;                                 \
            }

            CPPCMS_LOCALE_AS_MANIPULATOR(posix,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(number,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(currency,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(percent,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(date,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(time,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(datetime,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(strftime,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(spellout,display_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(ordinal,display_flags_mask)

            CPPCMS_LOCALE_AS_MANIPULATOR(currency_default,currency_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(currency_iso,currency_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(currency_national,currency_flags_mask)
            
            CPPCMS_LOCALE_AS_MANIPULATOR(time_default,time_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(time_short,time_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(time_medium,time_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(time_long,time_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(time_full,time_flags_mask)
            
            CPPCMS_LOCALE_AS_MANIPULATOR(date_default,date_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(date_short,date_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(date_medium,date_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(date_long,date_flags_mask)
            CPPCMS_LOCALE_AS_MANIPULATOR(date_full,date_flags_mask)

            
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

            inline details::set_timezone time_zone(cppcms::locale::time_zone const &id) 
            {
                details::set_timezone tz;
                tz.id=id.id();
                return tz;
            }



        } // as manipulators
        
        #undef CPPCMS_LOCALE_AS_MANIPULATOR



    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
