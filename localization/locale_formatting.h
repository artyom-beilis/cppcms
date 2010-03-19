//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_FORMATTING_HPP_INCLUDED
#define CPPCMS_LOCALE_FORMATTING_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include "locale_time_zone.h"
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

        /// \cond INTERNAL

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

        /// \endcond

        ///
        /// \brief This namespace includes all manipulators that can be used on IO streams
        ///
        namespace as {
            ///
            /// \defgroup manipulators I/O Stream manipulators
            ///
            /// @{
            ///

            ///
            /// Format values with "POSIX" or "C"  locale. Note, if locale was created with additional non-classic locale then
            /// These numbers may be localized
            ///
            
            inline std::ios_base & posix(std::ios_base & ios)
            {
                ext_setf(ios, flags::posix, flags::display_flags_mask);
                return ios;
            }

            ///
            /// Format a number. Note, unlike standard number formatting, integers would be treated like real numbers when std::fixed or
            /// std::scientific manipulators were applied
            ///
            inline std::ios_base & number(std::ios_base & ios)
            {
                ext_setf(ios, flags::number, flags::display_flags_mask);
                return ios;
            }
            
            ///
            /// Format currency, number is treated like amount of money
            ///
            inline std::ios_base & currency(std::ios_base & ios)
            {
                ext_setf(ios, flags::currency, flags::display_flags_mask);
                return ios;
            }
            
            ///
            /// Format percent, value 0.3 is treaded as 30%.
            ///
            inline std::ios_base & percent(std::ios_base & ios)
            {
                ext_setf(ios, flags::percent, flags::display_flags_mask);
                return ios;
            }
            
            ///
            /// Format a date, number is treaded as POSIX time
            ///
            inline std::ios_base & date(std::ios_base & ios)
            {
                ext_setf(ios, flags::date, flags::display_flags_mask);
                return ios;
            }

            ///
            /// Format a time, number is treaded as POSIX time
            ///
            inline std::ios_base & time(std::ios_base & ios)
            {
                ext_setf(ios, flags::time, flags::display_flags_mask);
                return ios;
            }

            ///
            /// Format a date and time, number is treaded as POSIX time
            ///
            inline std::ios_base & datetime(std::ios_base & ios)
            {
                ext_setf(ios, flags::datetime, flags::display_flags_mask);
                return ios;
            }

            ///
            /// Create formatted date time, Please note, this manipulator only changes formatting mode,
            /// and not format itself, so you are probably looking for ftime manipulator
            ///
            inline std::ios_base & strftime(std::ios_base & ios)
            {
                ext_setf(ios, flags::strftime, flags::display_flags_mask);
                return ios;
            }
            
            ///
            /// Spell the number, like "one hundred and ten"
            ///
            inline std::ios_base & spellout(std::ios_base & ios)
            {
                ext_setf(ios, flags::spellout, flags::display_flags_mask);
                return ios;
            }
            
            ///
            /// Write an order of the number like 4th.
            ///
            inline std::ios_base & ordinal(std::ios_base & ios)
            {
                ext_setf(ios, flags::ordinal, flags::display_flags_mask);
                return ios;
            }

            ///
            /// Set default currency formatting style -- national, like "$"
            ///
            inline std::ios_base & currency_default(std::ios_base & ios)
            {
                ext_setf(ios, flags::currency_default, flags::currency_flags_mask);
                return ios;
            }

            ///
            /// Set ISO currency formatting style, like "USD", (requires ICU >= 4.2)
            ///
            inline std::ios_base & currency_iso(std::ios_base & ios)
            {
                ext_setf(ios, flags::currency_iso, flags::currency_flags_mask);
                return ios;
            }

            ///
            /// Set national currency formatting style, like "$"
            ///
            inline std::ios_base & currency_national(std::ios_base & ios)
            {
                ext_setf(ios, flags::currency_national, flags::currency_flags_mask);
                return ios;
            }

            ///
            /// set default (medium) time formatting style
            ///
            inline std::ios_base & time_default(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_default, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set short time formatting style
            ///
            inline std::ios_base & time_short(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_short, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set medium time formatting style
            ///
            inline std::ios_base & time_medium(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_medium, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set long time formatting style
            ///
            inline std::ios_base & time_long(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_long, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set full time formatting style
            ///
            inline std::ios_base & time_full(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_full, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set default (medium) date formatting style
            ///
            inline std::ios_base & date_default(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_default, flags::date_flags_mask);
                return ios;
            }

            ///
            /// set short date formatting style
            ///
            inline std::ios_base & date_short(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_short, flags::date_flags_mask);
                return ios;
            }

            ///
            /// set medium date formatting style
            ///
            inline std::ios_base & date_medium(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_medium, flags::date_flags_mask);
                return ios;
            }

            ///
            /// set long date formatting style
            ///
            inline std::ios_base & date_long(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_long, flags::date_flags_mask);
                return ios;
            }

            ///
            /// set full date formatting style
            ///
            inline std::ios_base & date_full(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_full, flags::date_flags_mask);
                return ios;
            }            
            
            
            /// \cond INTERNAL 
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
            /// \endcond 

            ///
            /// Set strftime like formatting string
            ///
            /// Please note, formatting flags are very similar but not exactly the same as flags for C function strftime.
            /// Differences: some flags as "%e" do not add blanks to fill text up to two spaces, not all flags supported.
            ///
            /// Flags:
            /// -   "%a" -- Abbreviated  weekday (Sun.)
            /// -   "%A" -- Full weekday (Sunday)
            /// -   "%b" -- Abbreviated month (Jan.)
            /// -   "%B" -- Full month (January)
            /// -   "%c" -- Locale date-time format. **Note:** prefer using "as::datetime"
            /// -   "%d" -- Day of Month [01,31]
            /// -   "%e" -- Day of Month [1,31]
            /// -   "%h" -- Same as "%b"
            /// -   "%H" -- 24 clock hour [00,23]
            /// -   "%I" -- 12 clock hour [01,12]
            /// -   "%j" -- Day of year [1,366]
            /// -   "%m" -- Month [01,12]
            /// -   "%M" -- Minute [00,59]
            /// -   "%n" -- New Line
            /// -   "%p" -- AM/PM in locale representation
            /// -   "%r" -- Time with AM/PM, same as "%I:%M:%S %p"
            /// -   "%R" -- Same as "%H:%M"
            /// -   "%S" -- Second [00,61]
            /// -   "%t" -- Tab character
            /// -   "%T" -- Same as "%H:%M:%S"
            /// -   "%x" -- Local date representation. **Note:** prefer using "as::date"
            /// -   "%X" -- Local time representation. **Note:** prefer using "as::time"
            /// -   "%y" -- Year [00,99]
            /// -   "%Y" -- 4 digits year. (2009)
            /// -   "%Z" -- Time Zone
            /// -   "%%" -- Percent symbol
            ///


            template<typename CharType>
            details::add_ftime<CharType> ftime(std::basic_string<CharType> const &format)
            {
                details::add_ftime<CharType> fmt;
                fmt.ftime=format;
                return fmt;
            }

            ///
            /// See ftime(std::basic_string<CharType> const &format)
            ///
            template<typename CharType>
            details::add_ftime<CharType> ftime(CharType const *format)
            {
                details::add_ftime<CharType> fmt;
                fmt.ftime=format;
                return fmt;
            }

            /// \cond INTERNAL
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
            /// \endcond
            
            ///
            /// Set GMT time zone to stream
            /// 
            inline std::ios_base &gmt(std::ios_base &ios)
            {
                ext_pattern<char>(ios,flags::time_zone_id,"GMT");
                return ios;
            }

            ///
            /// Set local time zone to stream
            ///
            inline std::ios_base &local_time(std::ios_base &ios)
            {
                ext_pattern(ios,flags::time_zone_id,std::string());
                return ios;
            }

            ///
            /// Set time zone using \a id
            ///
            inline details::set_timezone time_zone(char const *id) 
            {
                details::set_timezone tz;
                tz.id=id;
                return tz;
            }

            ///
            /// Set time zone using \a id
            ///
            inline details::set_timezone time_zone(std::string const &id) 
            {
                details::set_timezone tz;
                tz.id=id;
                return tz;
            }

            ///
            /// Set time zone using time_zone class \a id
            ///
            inline details::set_timezone time_zone(cppcms::locale::time_zone const &id) 
            {
                details::set_timezone tz;
                tz.id=id.id();
                return tz;
            }


        ///
        /// @}
        ///

        } // as manipulators
        
    } // locale
} // boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
