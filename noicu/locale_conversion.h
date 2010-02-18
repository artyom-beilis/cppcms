//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
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

        namespace impl {
            typedef enum {
                upper_case,
                lower_case,
            } conversion_type;
            
            std::string CPPCMS_API convert(conversion_type how,char const *begin,char const *end,std::locale const &l);
        }

        ///
        /// Convert a string \a str to upper case according to locale \a loc
        ///

        template<typename CharType>
        std::basic_string<CharType> to_upper(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::upper_case,str.data(),str.data()+str.size(),loc);
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
            return impl::convert(impl::upper_case,str,end,loc);
        }
        
        ///
        /// Convert a string in range [begin,end) to upper case according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> to_upper(CharType const *begin,CharType const *end,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::upper_case,begin,end,loc);
        }

        ///////////////////////////////////////////////////
        
        ///
        /// Convert a string \a str to lower case according to locale \a loc
        ///

        template<typename CharType>
        std::basic_string<CharType> to_lower(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::lower_case,str.data(),str.data()+str.size(),loc);
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
            return impl::convert(impl::lower_case,str,end,loc);
        }
        
        ///
        /// Convert a string in range [begin,end) to lower case according to locale \a loc
        ///
        template<typename CharType>
        std::basic_string<CharType> to_lower(CharType const *begin,CharType const *end,std::locale const &loc=std::locale())
        {
            return impl::convert(impl::lower_case,begin,end,loc);
        }
        ///////////////////////////////////////////////////
        

    } // locale

} // boost


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

