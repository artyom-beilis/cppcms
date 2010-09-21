//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_IMPL_POSIX_LOCALIZATION_BACKEND_HPP
#define BOOSTER_LOCALE_IMPL_POSIX_LOCALIZATION_BACKEND_HPP
namespace booster {
    namespace locale {
        class localization_backend;
        namespace impl_posix { 
            localization_backend *create_localization_backend();
        } // impl_std
    } // locale 
} // boost
#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

