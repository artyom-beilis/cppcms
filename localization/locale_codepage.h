//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_CODEPAGE_HPP_INCLUDED
#define CPPCMS_LOCALE_CODEPAGE_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#include "locale_info.h"
#include "cstdint.h"
#include <stdexcept>

namespace cppcms {
    namespace locale {

        ///
        /// This function creates std::codecvt facet suitable for embedding in locale class for codepage conversion.
        ///
        /// Usually never called directly, generator class is used instead
        ///
        template<typename CharType>
        std::codecvt<CharType,char,mbstate_t> *create_codecvt(info const &inf);
        
        /// \cond INTERNAL 
        template<>
        CPPCMS_API std::codecvt<char,char,mbstate_t> *create_codecvt(info const &inf);

        #ifndef CPPCMS_NO_STD_WSTRING
        template<>
        CPPCMS_API std::codecvt<wchar_t,char,mbstate_t> *create_codecvt(info const &inf);
        #endif

        #ifdef CPPCMS_HAS_CHAR16_T
        template<>
        CPPCMS_API std::codecvt<char16_t,char,mbstate_t> *create_codecvt(info const &inf);
        #endif

        #ifdef CPPCMS_HAS_CHAR32_T
        template<>
        CPPCMS_API std::codecvt<char32_t,char,mbstate_t> *create_codecvt(info const &inf);
        #endif
        /// \endcond

        ///
        /// \brief Namespace that contains all functions related to character set conversion
        ///
        namespace conv {
            ///
            /// \defgroup codepage Character conversion functions
            ///
            /// @{

            ///
            /// \brief The excepton that is thrown in case of conversion error
            ///
            class conversion_error : public std::runtime_error {
            public:
                conversion_error() : std::runtime_error("Conversion failed") {}
            };
            

            ///
            /// enum that defines conversion policy
            ///
            typedef enum {
                skip            = 0,    ///< Skip illegal/unconvertable characters
                stop            = 1,    ///< Stop conversion and throw conversion_error
                default_method  = skip  ///< Default method - skip
            } method_type;

            ///
            /// convert string to UTF string from text in range [begin,end) encoded with \a charset according to policy \a how
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *begin,char const *end,std::string const &charset,method_type how=default_method);

            ///
            /// convert UTF text in range [begin,end) to a text encoded with \a charset according to policy \a how
            ///
            template<typename CharType>
            std::string from_utf(CharType const *begin,CharType const *end,std::string const &charset,method_type how=default_method);

            ///
            /// convert string to UTF string from text in range [begin,end) encoded according to locale \a loc according to policy \a how
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *begin,char const *end,std::locale const &loc,method_type how=default_method)
            {
                return to_utf<CharType>(begin,end,std::use_facet<info>(loc).encoding(),how);
            }

            ///
            /// convert UTF text in range [begin,end) to a text encoded according to locale \a loc according to policy \a how
            ///
            template<typename CharType>
            std::string from_utf(CharType const *begin,CharType const *end,std::locale const &loc,method_type how=default_method)
            {
                return from_utf(begin,end,std::use_facet<info>(loc).encoding(),how);
            }

            ///
            /// convert a string \a text encoded with \a charset to UTF string
            ///
            
            template<typename CharType>
            std::basic_string<CharType> to_utf(std::string const &text,std::string const &charset,method_type how=default_method)
            {
                return to_utf<CharType>(text.c_str(),text.c_str()+text.size(),charset,how);
            }

            ///
            /// Convert a \a text from \a charset to UTF string
            ///
            template<typename CharType>
            std::string from_utf(std::basic_string<CharType> const &text,std::string const &charset,method_type how=default_method)
            {
                return from_utf(text.c_str(),text.c_str()+text.size(),charset,how);
            }

            ///
            /// Convert a \a text from \a charset to UTF string
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *text,std::string const &charset,method_type how=default_method)
            {
                char const *text_end = text;
                while(*text_end) 
                    text_end++;
                return to_utf<CharType>(text,text_end,charset,how);
            }

            ///
            /// Convert a \a text from UTF to \a charset
            ///
            template<typename CharType>
            std::string from_utf(CharType const *text,std::string const &charset,method_type how=default_method)
            {
                CharType const *text_end = text;
                while(*text_end) 
                    text_end++;
                return from_utf(text,text_end,charset,how);
            }

            ///
            /// Convert a \a text in locale encoding given by \a loc to UTF
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(std::string const &text,std::locale const &loc,method_type how=default_method)
            {
                return to_utf<CharType>(text.c_str(),text.c_str()+text.size(),loc,how);
            }

            ///
            /// Convert a \a text in UTF to locale encoding given by \a loc
            ///
            template<typename CharType>
            std::string from_utf(std::basic_string<CharType> const &text,std::locale const &loc,method_type how=default_method)
            {
                return from_utf(text.c_str(),text.c_str()+text.size(),loc,how);
            }

            ///
            /// Convert a \a text in locale encoding given by \a loc to UTF
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *text,std::locale const &loc,method_type how=default_method)
            {
                char const *text_end = text;
                while(*text_end) 
                    text_end++;
                return to_utf<CharType>(text,text_end,loc,how);
            }

            ///
            /// Convert a \a text in UTF to locale encoding given by \a loc
            ///
            template<typename CharType>
            std::string from_utf(CharType const *text,std::locale const &loc,method_type how=default_method)
            {
                CharType const *text_end = text;
                while(*text_end) 
                    text_end++;
                return from_utf(text,text_end,loc,how);
            }

            /// @}

            /// \cond INTERNAL

            template<>
            CPPCMS_API std::basic_string<char> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            CPPCMS_API std::string from_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            #ifndef CPPCMS_NO_STD_WSTRING
            template<>
            CPPCMS_API std::basic_string<wchar_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            CPPCMS_API std::string from_utf(wchar_t const *begin,wchar_t const *end,std::string const &charset,method_type how);
            #endif

            #ifdef CPPCMS_HAS_CHAR16_T
            template<>
            CPPCMS_API std::basic_string<char16_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            CPPCMS_API std::string from_utf(char16_t const *begin,char16_t const *end,std::string const &charset,method_type how);
            #endif

            #ifdef CPPCMS_HAS_CHAR32_T
            template<>
            CPPCMS_API std::basic_string<char32_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            CPPCMS_API std::string from_utf(char32_t const *begin,char32_t const *end,std::string const &charset,method_type how);
            #endif

            /// \endcond
        } // conv

    } // locale
} // boost


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

