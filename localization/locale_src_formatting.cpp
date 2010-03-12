//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_formatting.h"
#include "locale_src_formatter.hpp"
#include <typeinfo>
#include "locale_src_formatting_info.hpp"
#include "locale_src_ios_prop.hpp"

namespace cppcms {
    namespace locale {

        namespace {
            impl::ios_info &info(std::ios_base &ios)
            {
                return impl::ios_prop<impl::ios_info>::get(ios);
            }

            template<typename Char>
            void do_ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::basic_string<Char> const &pattern)
            {
                switch(pattern_id) {
                case flags::datetime_pattern:
                    info(ios).datetime(pattern);
                    break;
                default:
                    throw std::bad_cast();
                }
            }
            
            template<typename Char>
            std::basic_string<Char> do_ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
            {
                switch(pattern_id) {
                case flags::datetime_pattern:
                    return info(ios).datetime<Char>();
                default:
                    throw std::bad_cast();
                }
            }
        }

        CPPCMS_API uint64_t ext_flags(std::ios_base &ios)
        {
            return info(ios).flags();
        }
        CPPCMS_API uint64_t ext_flags(std::ios_base &ios,flags::display_flags_type mask)
        {
            return info(ios).flags() & mask;
        }
        
        CPPCMS_API void ext_setf(std::ios_base &ios,flags::display_flags_type flags,flags::display_flags_type mask)
        {
            impl::ios_info &inf=info(ios);
            inf.flags((inf.flags() & ~(uint64_t(mask))) | flags);
        }
        
        CPPCMS_API int ext_value(std::ios_base &ios,flags::value_type id)
        {
            impl::ios_info &inf=info(ios);
            return inf.value(id);
        }
        CPPCMS_API void ext_value(std::ios_base &ios,flags::value_type id,int value)
        {
            impl::ios_info &inf=info(ios);
            inf.value(id,value);
        }


        template<>
        CPPCMS_API void ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::string const &pattern)
        {
            if(pattern_id == flags::time_zone_id)
                info(ios).timezone(pattern);
            else
                do_ext_pattern(ios,pattern_id,pattern);
        }
        
        template<>
        CPPCMS_API std::string ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
        {
            if(pattern_id == flags::time_zone_id)
                return info(ios).timezone();
            else
                return do_ext_pattern<char>(ios,pattern_id);
        }

        #ifndef CPPCMS_NO_STD_WSTRING
        
        template<>
        CPPCMS_API void ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::wstring const &pattern)
        {
            do_ext_pattern(ios,pattern_id,pattern);
        }

        template<>
        CPPCMS_API std::wstring ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
        {
            return do_ext_pattern<wchar_t>(ios,pattern_id);
        }

        #endif // CPPCMS_NO_STD_WSTRING

        #ifdef CPPCMS_HAS_CHAR16_T
        template<>
        CPPCMS_API void ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::u16string const &pattern)
        {
            do_ext_pattern(ios,pattern_id,pattern);
        }

        template<>
        CPPCMS_API std::u16string ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
        {
            return do_ext_pattern<char16_t>(ios,pattern_id);
        }
        #endif // char16_t, u16string

        #ifdef CPPCMS_HAS_CHAR32_T
        template<>
        CPPCMS_API void ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::u32string const &pattern)
        {
            do_ext_pattern(ios,pattern_id,pattern);
        }

        template<>
        CPPCMS_API std::u32string ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
        {
            return do_ext_pattern<char32_t>(ios,pattern_id);
        }
        #endif // char32_t, u32string
    } // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
