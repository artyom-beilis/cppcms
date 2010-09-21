//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <locale>
#include <string>
#include <ios>
#include <booster/locale/generator.h>
#include <booster/locale/info.h>
#include <booster/locale/util.h>
#include <sstream>
#include <stdlib.h>

#include "locale_data.h"

namespace booster {
namespace locale {
namespace util {
    class simple_info : public info {
    public:
        simple_info(std::string const &name,size_t refs = 0) :
            info(refs),
            name_(name)
        {
            d.parse(name);
        }
        virtual std::string get_string_property(string_propery v) const
        {
            switch(v) {
            case language_property:
                return d.language;
            case country_property:
                return d.country;
            case variant_property:
                return d.variant;
            case encoding_property:
                return d.encoding;
            case name_property:
                return name_;
            default:
                return "";
            };
        }

        virtual int get_ineger_property(integer_property v) const
        {
            switch(v) {
            case utf8_property:
                return d.utf8;
            default:
                return 0;
            }
        }
    private:
        locale_data d;
        std::string name_;
    };
    
    std::locale create_info(std::locale const &in,std::string const &name)
    {
        return std::locale(in,new simple_info(name));
    }


} // util
} // locale 
} //boost



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
