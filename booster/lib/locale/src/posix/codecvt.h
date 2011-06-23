//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_IMPL_POSIX_CODECVT_HPP
#define BOOSTER_LOCALE_IMPL_POSIX_CODECVT_HPP
#include <booster/config.h>
#include <booster/locale/util.h>

#include <memory>
#include <string>

namespace booster {
namespace locale {
namespace impl_posix {
    BOOSTER_API
    std::auto_ptr<util::base_converter> create_iconv_converter(std::string const &encoding);

} // impl_posix
} // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

