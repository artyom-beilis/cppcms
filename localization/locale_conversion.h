//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_CONVERTER_HPP_INCLUDED
#define CPPCMS_LOCALE_CONVERTER_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#include <locale>


namespace cppcms {
    namespace locale {
        
        ///
        /// \defgroup convert Text Conversions 
        ///
        ///  This module provides various function for string manipulation like Unicode normalization, case conversion etc.
        /// @{
        ///

        /// \cond INTERNAL
        namespace impl {
            typedef enum {
                normalization,
                upper_case,
                lower_case,
                case_folding,
                title_case
            } conversion_type;

            CPPCMS_API std::string convert(conversion_type how,char const *begin,char const *end,int flags,std::locale const *loc=0);
            #ifndef CPPCMS_NO_STD_WSTRING
            CPPCMS_API std::wstring convert(conversion_type how,wchar_t const *begin,wchar_t const *end,int flags,std::locale const *loc=0);
            #endif
            #ifdef CPPCMS_HAS_CHAR16_T
            CPPCMS_API std::u16string convert(conversion_type how,char16_t const *begin,char16_t const *end,int flags,std::locale const *loc=0);
            #endif
            #ifdef CPPCMS_HAS_CHAR32_T
            CPPCMS_API std::u32string convert(conversion_type how,char32_t const *begin,char32_t const *end,int flags,std::locale const *loc=0);
            #endif

        } // impl
        /// \endcond
 
        ///
        /// Type of normalization
        ///

        typedef enum {
            norm_nfd,   ///< Canonical decomposition
            norm_nfc,   ///< Canonical decomposition followed by canonical composition
            norm_nfkd,  ///< Compatibility decomposition
            norm_nfkc,  ///< Compatibility decomposition followed by canonical composition.
            norm_default = norm_nfc, ///< Default normalization - canonical decomposition followed by canonical composition
        } norm_type;
       
        ///
        /// Normalize Unicode string \a str according to normalization mode \a n
        ///
        /// Note: This function receives only Unicode strings, i.e.: UTF-8, UTF-16 or UTF-32. It does not takes
        /// in account the locale encoding, because Unicode decomposition and composition are meaningless outside 
        /// of Unicode character set.
        /// 
        template<typename CharType>
        std::basic_string<CharType> normalize(std::basic_string<CharType> const &str,norm_type n=norm_default)
        {
            return impl::convert(impl::normalization,str.data(),str.data() + str.size(),n);
        }

        ///
        /// Normalize NUL terminated Unicode string \a str according to normalization mode \a n
        ///
        /// Note: This function receives only Unicode strings, i.e.: UTF-8, UTF-16 or UTF-32. It does not takes
        /// in account the locale encoding, because Unicode decomposition and composition are meaningless outside 
        /// of Unicode character set.
        /// 
        template<typename CharType>
        std::basic_string<CharType> normalize(CharType const *str,norm_type n=norm_default)
        {
            CharType const *end=str;
            while(*end)
                end++;
            return impl::convert(impl::normalization,str,end,n);
        }
        
        ///
        /// Normalize Unicode string in range [begin,end) according to normalization mode \a n
        ///
        /// Note: This function receives only Unicode strings, i.e.: UTF-8, UTF-16 or UTF-32. It does not takes
        /// in account the locale encoding, because Unicode decomposition and composition are meaningless outside 
        /// of Unicode character set.
        /// 
        template<typename CharType>
        std::basic_string<CharType> normalize(CharType const *begin,CharType const *end,norm_type n=norm_default)
        {
            return impl::convert(impl::normalization,begin,end,n);
        }

        ///////////////////////////////////////////////////
        
        ///
        /// Convert a string \a str to upper case according to locale \a loc
        ///

        template<typename CharType>
        std::basic_string<CharType> to_upper(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::upper_case,str.data(),str.data()+str.size(),0,&loc);
        }
        
        ///
        /// Convert a NUL terminated string \a str to upper case according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> to_upper(CharType const *str,std::locale const &loc=std::locale())
        {
            CharType const *end=str;
            while(*end)
                end++;
            return impl::convert(impl::upper_case,str,end,0,&loc);
        }
        
        ///
        /// Convert a string in range [begin,end) to upper case according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> to_upper(CharType const *begin,CharType const *end,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::upper_case,begin,end,0,&loc);
        }

        ///////////////////////////////////////////////////
        
        ///
        /// Convert a string \a str to lower case according to locale \a loc
        ///

        template<typename CharType>
        std::basic_string<CharType> to_lower(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::lower_case,str.data(),str.data()+str.size(),0,&loc);
        }
        
        ///
        /// Convert a NUL terminated string \a str to lower case according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> to_lower(CharType const *str,std::locale const &loc=std::locale())
        {
            CharType const *end=str;
            while(*end)
                end++;
            return impl::convert(impl::lower_case,str,end,0,&loc);
        }
        
        ///
        /// Convert a string in range [begin,end) to lower case according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> to_lower(CharType const *begin,CharType const *end,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::lower_case,begin,end,0,&loc);
        }
        ///////////////////////////////////////////////////
        
        ///
        /// Convert a string \a str to title case according to locale \a loc
        ///

        template<typename CharType>
        std::basic_string<CharType> to_title(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::title_case,str.data(),str.data()+str.size(),0,&loc);
        }
        
        ///
        /// Convert a NUL terminated string \a str to title case according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> to_title(CharType const *str,std::locale const &loc=std::locale())
        {
            CharType const *end=str;
            while(*end)
                end++;
            return impl::convert(impl::title_case,str,end,0,&loc);
        }
        
        ///
        /// Convert a string in range [begin,end) to title case according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> to_title(CharType const *begin,CharType const *end,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::title_case,begin,end,0,&loc);
        }

        ///////////////////////////////////////////////////
        
        ///
        /// Fold case of a string \a str according to locale \a loc
        ///

        template<typename CharType>
        std::basic_string<CharType> fold_case(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::case_folding,str.data(),str.data()+str.size(),0,&loc);
        }
        
        ///
        /// Fold case of a NUL terminated string \a str according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> fold_case(CharType const *str,std::locale const &loc=std::locale())
        {
            CharType const *end=str;
            while(*end)
                end++;
            return impl::convert(impl::case_folding,str,end,0,&loc);
        }
        
        ///
        /// Fold case of a string in range [begin,end) according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> fold_case(CharType const *begin,CharType const *end,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::case_folding,begin,end,0,&loc);
        }

        ///
        ///@}
        ///
    } // locale

} // boost


#endif

///
/// \example conversions.cpp
///
/// Example of using various text conversion functions.
///
/// \example wconversions.cpp
///
/// Example of using various text conversion functions with wide strings.
///

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

