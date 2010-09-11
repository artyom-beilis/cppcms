//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_FORMATTER_H_INCLUDED
#define BOOSTER_LOCALE_FORMATTER_H_INCLUDED

#include <string>
#include <memory>
#include <booster/cstdint.h>
#include <booster/config.h>
#include <unicode/locid.h>

namespace booster {
namespace locale {
namespace impl_icu {        

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
        static std::auto_ptr<formatter> create(std::ios_base &ios,icu::Locale const &l,std::string const &enc);

        virtual ~formatter()
        {
        }
    }; // class formatter
    
    ///
    /// Specialization for real implementation
    ///
    template<>
    std::auto_ptr<formatter<char> > formatter<char>::create(std::ios_base &ios,icu::Locale const &l,std::string const &enc);

    ///
    /// Specialization for real implementation
    ///
    template<>
    std::auto_ptr<formatter<wchar_t> > formatter<wchar_t>::create(std::ios_base &ios,icu::Locale const &l,std::string const &e);

    #ifdef BOOSTER_HAS_CHAR16_T
    ///
    /// Specialization for real implementation
    ///
    template<>
    std::auto_ptr<formatter<char16_t> > formatter<char16_t>::create(std::ios_base &ios,icu::Locale const &l,std::string const &e);
    #endif

    #ifdef BOOSTER_HAS_CHAR32_T
    ///
    /// Specialization for real implementation
    ///
    template<>
    std::auto_ptr<formatter<char32_t> > formatter<char32_t>::create(std::ios_base &ios,icu::Locale const &l,std::string const &e);
    #endif

} // namespace impl_icu
} // namespace locale
} // namespace boost



#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
