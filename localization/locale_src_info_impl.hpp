//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_SRC_ICU_IMPL_HPP_INCLUDED
#define CPPCMS_LOCALE_SRC_ICU_IMPL_HPP_INCLUDED

#include <string>
#include <unicode/locid.h>
namespace cppcms {
    namespace locale {
        struct info_impl {
            icu::Locale locale;
            std::string encoding;
        };
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

