//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_TIME_ZONE_H_INCLUDED
#define BOOSTER_LOCALE_TIME_ZONE_H_INCLUDED

#include <booster/config.h>
#ifdef BOOSTER_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif

#include <string>


namespace booster {
    namespace locale {
        ///
        /// \addtogroup date_time
        ///
        /// @{

        ///
        /// \brief namespace that holds function for operating global time zone identifier
        ///
        namespace time_zone {
            ///
            /// Get global time zone identifier. If empty, system time zone is used
            ///
            BOOSTER_API std::string global();
            ///
            /// Set global time zone identifier returing pervious one. If empty, system time zone is used
            ///
            BOOSTER_API std::string global(std::string const &new_tz);
        }

        /// @}

    } // locale
} // boost

#ifdef BOOSTER_MSVC
#pragma warning(pop)
#endif


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
