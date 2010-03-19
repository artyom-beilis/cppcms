//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_COLLATOR_HPP_INCLUDED
#define CPPCMS_LOCALE_COLLATOR_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <locale>


namespace cppcms {
namespace locale {

    class info;

    ///
    /// \defgroup collation Collation 
    ///
    /// This module that introduces collation related classes
    ///
    /// @{

    ///
    /// \brief a base class that included collation level flags
    ///

    class collator_base {
    public:
        ///
        /// Unicode collation level types
        ///
        typedef enum {
            primary     = 0, ///< 1st collation level: base letters
            secondary   = 1, ///< 2nd collation level: letters and accents
            tertiary    = 2, ///< 3rd collation level: letters, accents and case
            quaternary  = 3, ///< 4th collation level: letters, accents, case and punctuation
            identical   = 4  ///< identical collation level: include code-point comparison
        } level_type;
    };
    
    ///
    /// \brief Collation facet. 
    ///
    /// It reimplements standard C++ stc::collate
    /// allowing usage of std::locale class for direct string comparison
    ///
    template<typename CharType>
    class collator : 
        public std::collate<CharType>,
        public collator_base
    {
    public:
        ///
        /// Type of underlying character
        ///
        typedef CharType char_type;
        ///
        /// Type of string used with this facet
        ///
        typedef std::basic_string<CharType> string_type;
        

        ///
        /// Compare two strings in rage [b1,e1), [b2,e2) according using a collation level \a level. Calls do_compare
        ///
        int compare(level_type level,
                    char_type const *b1,char_type const *e1,
                    char_type const *b2,char_type const *e2) const
        {
            return do_compare(level,b1,e1,b2,e2);
        }
        ///
        /// Create a binary string that can be compared to other in order to get collation order. The string is created
        /// for text in range [b,e). It is useful for collation of multiple strings for text. Calls do_transform
        ///
        string_type transform(level_type level,char_type const *b,char_type const *e) const
        {
            return do_transform(level,b,e);
        }

        ///
        /// Calculate a hash of a text in range [b,e). The value can be used for collation sensitive string comparison. Calls do_hash
        ///
        long hash(level_type level,char_type const *b,char_type const *e) const
        {
            return do_hash(level,b,e);
        }

        ///
        /// Compare two strings \a l and \a r using collation level \a level
        ///
        int compare(level_type level,string_type const &l,string_type const &r) const
        {
            return do_compare(level,l.data(),l.data()+l.size(),r.data(),r.data()+r.size());
        }

        ///
        /// Calculate a hash that can be used for collation sensitive string comparison of a string \a s
        ///

        long hash(level_type level,string_type const &s) const
        {
            return do_hash(level,s.data(),s.data()+s.size());
        }
        ///
        /// Create a binary string from string \a s, that can be compared to other, useful for collation of multiple
        /// strings.
        ///
        string_type transform(level_type level,string_type const &s) const
        {
            return do_transform(level,s.data(),s.data()+s.size());
        }

        ///
        /// A static member used for creation of collator instances, generally called by a generator class.
        /// 
        static collator<CharType> *create(info const &inf);
        
    protected:

        ///
        /// constructor of the collator object
        ///
        collator(size_t refs = 0) : std::collate<CharType>(refs) 
        {
        }

        virtual ~collator()
        {
        }
        
        ///
        /// This function is used to override default collation function that does not take in account collation level.
        /// Uses primary level
        ///
        virtual int do_compare( char_type const *b1,char_type const *e1,
                                char_type const *b2,char_type const *e2) const
        {
            return do_compare(primary,b1,e1,b2,e2);
        }
        ///
        /// This function is used to override default collation function that does not take in account collation level.
        /// Uses primary level
        ///
        virtual string_type do_transform(char_type const *b,char_type const *e) const
        {
            return do_transform(primary,b,e);
        }
        ///
        /// This function is used to override default collation function that does not take in account collation level.
        /// Uses primary level
        ///
        virtual long do_hash(char_type const *b,char_type const *e) const
        {
            return do_hash(primary,b,e);
        }

        ///
        /// Actual function that performs comparison between the strings. For details see compare member function. Can be overridden. 
        ///
        virtual int do_compare( level_type level,
                                char_type const *b1,char_type const *e1,
                                char_type const *b2,char_type const *e2) const = 0;
        ///
        /// Actual function that performs transformation. For details see transform member function. Can be overridden. 
        ///
        virtual string_type do_transform(level_type level,char_type const *b,char_type const *e) const = 0;
        ///
        /// Actual function that calculates hash. For details see hash member function. Can be overridden. 
        ///
        virtual long do_hash(level_type level,char_type const *b,char_type const *e) const = 0;


    };

    /// \cond INTERNAL 

    template<>
    CPPCMS_API collator<char> *collator<char>::create(info const &inf);
    #ifndef CPPCMS_NO_STD_WSTRING
    template<>
    CPPCMS_API collator<wchar_t> *collator<wchar_t>::create(info const &inf);
    #endif
    
    #ifdef CPPCMS_HAS_CHAR16_T
    template<>
    CPPCMS_API collator<char16_t> *collator<char16_t>::create(info const &inf);
    #endif
    
    #ifdef CPPCMS_HAS_CHAR32_T
    template<>
    CPPCMS_API collator<char32_t> *collator<char32_t>::create(info const &inf);
    #endif
    /// \endcond

    ///
    /// \brief This class can be used in STL algorithms and containers for comparison of strings
    /// with different level then primary
    ///
    /// For example:
    ///
    /// \code
    ///  std::map<std::string,std::string,comparator<char,collator_base::secondary> > data;
    /// \endcode
    /// 
    /// Would create a map the keys of which are sorted using secondary collation level
    ///
    template<typename CharType,collator_base::level_type default_level = collator_base::primary>
    struct comparator
    {
    public:
        ///
        /// Create a comparator class for locale \a l and with collation leval \a level
        ///
        comparator(std::locale const &l=std::locale(),collator_base::level_type level=default_level) : 
            locale_(l),
            level_(level)
        {
        }

        ///
        /// Compare two strings -- equivalent to return left < right according to collation rules
        ///
        bool operator()(std::basic_string<CharType> const &left,std::basic_string<CharType> const &right) const
        {
            return std::use_facet<collator<CharType> >(locale_).compare(level_,left,right) < 0;
        }
    private:
        std::locale locale_;
        collator_base::level_type level_;
    };


    ///
    ///@}
    ///

    } // locale
} // boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif
///
/// \example collate.cpp
/// Example of using collation functions
///
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
