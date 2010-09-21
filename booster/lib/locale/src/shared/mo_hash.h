//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <booster/cstdint.h>

namespace booster {
    namespace locale {
        namespace gnu_gettext {

            inline uint32_t pj_winberger_hash_function(char const *ptr,char const *end = 0)
            {
                uint32_t value=0;
                if(end == 0) {
                    end = ptr;
                    while(*end)
                        end++;
                }

                for(;ptr!=end;ptr++) {
                    value = (value << 4) + static_cast<unsigned char>(*ptr);
                    uint32_t high = (value & 0xF0000000U);
                    if(high!=0)
                        value = (value ^ (high >> 24)) ^ high;
                }
                return value;
            }
        }
    }
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

