//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/locale/boundary.h>
#include <booster/locale/collator.h>
#include <booster/locale/conversion.h>
#include <booster/locale/date_time_facet.h>
#include <booster/locale/message.h>
#include <booster/locale/info.h>

namespace booster {
    namespace locale {

        std::locale::id info::id;
        std::locale::id calendar_facet::id;

        std::locale::id converter<char>::id;
        std::locale::id base_message_format<char>::id;

        std::locale::id converter<wchar_t>::id;
        std::locale::id base_message_format<wchar_t>::id;

        #ifdef BOOSTER_HAS_CHAR16_T

        std::locale::id converter<char16_t>::id;
        std::locale::id base_message_format<char16_t>::id;

        #endif

        #ifdef BOOSTER_HAS_CHAR32_T

        std::locale::id converter<char32_t>::id;
        std::locale::id base_message_format<char32_t>::id;

        #endif

        namespace boundary {        

            std::locale::id boundary_indexing<char>::id;

            std::locale::id boundary_indexing<wchar_t>::id;

            #ifdef BOOSTER_HAS_CHAR16_T
            std::locale::id boundary_indexing<char16_t>::id;
            #endif

            #ifdef BOOSTER_HAS_CHAR32_T
            std::locale::id boundary_indexing<char32_t>::id;
            #endif
        }

        namespace {
            struct install_all {
                install_all()
                {
                    std::locale l = std::locale::classic();
                    install_by<char>();
                    install_by<wchar_t>();
                    #ifdef BOOSTER_HAS_CHAR16_T
                    install_by<char16_t>();
                    #endif
                    #ifdef BOOSTER_HAS_CHAR32_T
                    install_by<char32_t>();
                    #endif

                    std::has_facet<info>(l);
                    std::has_facet<calendar_facet>(l);
                }
                template<typename Char>
                void install_by()
                {
                    std::locale l = std::locale::classic();
                    std::has_facet<boundary::boundary_indexing<Char> >(l);
                    std::has_facet<converter<Char> >(l);
                    std::has_facet<base_message_format<Char> >(l);
                }
            } installer;
        }

    }
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
