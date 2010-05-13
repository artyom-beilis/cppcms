//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include <cppcms/locale_formatting.h>
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

    } // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
