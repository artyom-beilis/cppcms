//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_FORMATTER_HPP_INCLUDED
#define CPPCMS_LOCALE_FORMATTER_HPP_INCLUDED

#include <string>
#include <memory>
#include "cstdint.h"
#include "defs.h"
#include "config.h"

namespace cppcms {
    namespace locale {

        ///
        /// \brief Special base polymorphic class that used as character type independent base for all formatter classes
        ///

        class base_formatter {
        public:
            virtual ~base_formatter()
            {
            }
        };

        ///
        /// \brief A class that used for formatting of numbers, currency and dates/times
        ///
        template<typename CharType>
        class formatter : public base_formatter {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;

            ///
            /// Format the value and return number of Unicode code points
            ///
            virtual string_type format(double value,size_t &code_points) const = 0;
            ///
            /// Format the value and return number of Unicode code points
            ///
            virtual string_type format(int64_t value,size_t &code_points) const = 0;
            ///
            /// Format the value and return number of Unicode code points
            ///
            virtual string_type format(uint64_t value,size_t &code_points) const = 0;
            ///
            /// Format the value and return number of Unicode code points
            ///
            virtual string_type format(int32_t value,size_t &code_points) const = 0;
            ///
            /// Format the value and return number of Unicode code points
            ///
            virtual string_type format(uint32_t value,size_t &code_points) const = 0;

            ///
            /// Parse the string and return number of used characters. If returns 0
            /// then parsing failed.
            ///
            virtual size_t parse(string_type const &str,double &value) const = 0;
            ///
            /// Parse the string and return number of used characters. If returns 0
            /// then parsing failed.
            ///
            virtual size_t parse(string_type const &str,int64_t &value) const = 0;
            ///
            /// Parse the string and return number of used characters. If returns 0
            /// then parsing failed.
            ///
            virtual size_t parse(string_type const &str,uint64_t &value) const = 0;
            ///
            /// Parse the string and return number of used characters. If returns 0
            /// then parsing failed.
            ///
            virtual size_t parse(string_type const &str,int32_t &value) const = 0;
            ///
            /// Parse the string and return number of used characters. If returns 0
            /// then parsing failed.
            ///
            virtual size_t parse(string_type const &str,uint32_t &value) const = 0;

            ///
            /// Get formatter for current state of ios_base -- flags and locale,
            /// NULL may be returned if invalid combination of flags provided or this type
            /// of formatting is not supported by locale. See: create
            ///
            /// Note: formatter is cached. If \a ios is not changed (no flags or locale changed)
            /// the formatter would remain the same. Otherwise it would be rebuild and cached
            /// for future use. It is usefull for saving time for generation
            /// of multiple values with same locale.
            ///
            /// For example, this code:
            ///
            /// \code
            ///     std::cout << as::spellout;
            ///     for(int i=1;i<=10;i++)
            ///         std::cout << i <<std::endl;
            /// \endcode
            ///
            /// Would create new spelling formatter only once.
            ///
            static formatter const *get(std::ios_base &ios);

            ///
            /// Create formatter for current state of ios_base -- flags and locale,
            /// auto_ptr may hold NULL pointer if the formatting is not supported by locale
            /// or invalid combination of flags was given. User must check if auto_ptr::get
            /// returns non-zero. Otherwise it should fallback to standard C++ methods.
            ///
            /// Note: returning NULL is **not** a error. If no locale specific flags are given
            /// no formatter would be generated! 
            ///
            static std::auto_ptr<formatter> create(std::ios_base &ios);

            virtual ~formatter()
            {
            }
        };
        
        ///
        /// Specialization for real implementation
        ///
        template<>
        CPPCMS_API std::auto_ptr<formatter<char> > formatter<char>::create(std::ios_base &ios);

        ///
        /// Specialization for real implementation
        ///
        template<>
        CPPCMS_API formatter<char> const *formatter<char>::get(std::ios_base &ios);

        #ifndef CPPCMS_NO_STD_WSTRING
        ///
        /// Specialization for real implementation
        ///
        template<>
        CPPCMS_API std::auto_ptr<formatter<wchar_t> > formatter<wchar_t>::create(std::ios_base &ios);
        ///
        /// Specialization for real implementation
        ///
        template<>
        CPPCMS_API formatter<wchar_t> const *formatter<wchar_t>::get(std::ios_base &ios);
        #endif

        #ifdef CPPCMS_HAS_CHAR16_T
        ///
        /// Specialization for real implementation
        ///
        template<>
        CPPCMS_API std::auto_ptr<formatter<char16_t> > formatter<char16_t>::create(std::ios_base &ios);
        ///
        /// Specialization for real implementation
        ///
        template<>
        CPPCMS_API formatter<char16_t> const *formatter<char16_t>::get(std::ios_base &ios);
        #endif

        #ifdef CPPCMS_HAS_CHAR32_T
        ///
        /// Specialization for real implementation
        ///
        template<>
        CPPCMS_API std::auto_ptr<formatter<char32_t> > formatter<char32_t>::create(std::ios_base &ios);
        ///
        /// Specialization for real implementation
        ///
        template<>
        CPPCMS_API formatter<char32_t> const *formatter<char32_t>::get(std::ios_base &ios);
        #endif

    } // namespace locale
} // namespace boost



#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
