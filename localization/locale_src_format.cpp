//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_format.h"
#include "locale_generator.h"
#include "locale_info.h"
#include <limits>
#include "locale_src_formatting_info.hpp"
#include <stdlib.h>

#include <iostream>

namespace cppcms {
    namespace locale {
        namespace details {
            struct format_parser::data {
                unsigned position;
                std::streamsize precision;
                std::ios_base::fmtflags flags;
                impl::ios_info info;
                std::locale saved_locale;
                bool restore_locale;
            };

            format_parser::format_parser(std::ios_base &ios) : 
                ios_(ios),
                d(new data)
            {
                d->position=std::numeric_limits<unsigned>::max();
                d->precision=ios.precision();
                d->info=impl::ios_prop<impl::ios_info>::get(ios);
                d->saved_locale = ios.getloc();
                d->restore_locale=false;
            }

            format_parser::~format_parser()
            {
            }

            void format_parser::restore()
            {
                impl::ios_prop<impl::ios_info>::set(d->info,ios_);
                ios_.width(0);
                if(d->restore_locale)
                    ios_.imbue(d->saved_locale);
            }

            unsigned format_parser::get_posision()
            {
                return d->position;
            }

            void format_parser::set_one_flag(std::string const &key,std::string const &value)
            {
                if(key.empty())
                    return;
                unsigned i;
                for(i=0;i<key.size();i++) {
                    if(key[i] < '0' || '9'< key[i])
                        break;
                }
                if(i==key.size()) {
                    d->position=atoi(key.c_str()) - 1;
                    return;
                }

                if(key=="num" || key=="number") {
                    as::number(ios_);

                    if(value=="hex")
                        ios_.setf(std::ios_base::hex,std::ios_base::basefield);
                    else if(value=="oct")
                        ios_.setf(std::ios_base::oct,std::ios_base::basefield);
                    else if(value=="sci" || value=="scientific")
                        ios_.setf(std::ios_base::scientific,std::ios_base::floatfield);
                }
                else if(key=="cur" || key=="currency") {
                    as::currency(ios_);
                    if(value=="iso") 
                        as::currency_iso(ios_);
                    else if(value=="nat" || value=="national")
                        as::currency_national(ios_);
                }
                else if(key=="per" || key=="percent") {
                    as::percent(ios_);
                }
                else if(key=="date") {
                    as::date(ios_);
                    if(value=="s" || value=="short")
                        as::date_short(ios_);
                    else if(value=="m" || value=="medium")
                        as::date_medium(ios_);
                    else if(value=="l" || value=="long")
                        as::date_long(ios_);
                    else if(value=="f" || value=="full")
                        as::date_full(ios_);
                }
                else if(key=="time") {
                    as::time(ios_);
                    if(value=="s" || value=="short")
                        as::time_short(ios_);
                    else if(value=="m" || value=="medium")
                        as::time_medium(ios_);
                    else if(value=="l" || value=="long")
                        as::time_long(ios_);
                    else if(value=="f" || value=="full")
                        as::time_full(ios_);
                }
                else if(key=="dt" || key=="datetime") {
                    as::datetime(ios_);
                    if(value=="s" || value=="short") {
                        as::date_short(ios_);
                        as::time_short(ios_);
                    }
                    else if(value=="m" || value=="medium") {
                        as::date_medium(ios_);
                        as::time_medium(ios_);
                    }
                    else if(value=="l" || value=="long") {
                        as::date_long(ios_);
                        as::time_long(ios_);
                    }
                    else if(value=="f" || value=="full") {
                        as::date_full(ios_);
                        as::time_full(ios_);
                    }
                }
                else if(key=="spell" || key=="spellout") {
                    as::spellout(ios_);
                }
                else if(key=="ord" || key=="ordinal") {
                    as::ordinal(ios_);
                }
                else if(key=="left" || key=="<")
                    ios_.setf(std::ios_base::left,std::ios_base::adjustfield);
                else if(key=="right" || key==">")
                    ios_.setf(std::ios_base::right,std::ios_base::adjustfield);
                else if(key=="gmt")
                    as::gmt(ios_);
                else if(key=="local")
                    as::local_time(ios_);
                else if(key=="timezone" || key=="tz")
                    ext_pattern(ios_,flags::time_zone_id,value);
                else if(key=="w" || key=="width")
                    ios_.width(atoi(value.c_str()));
                else if(key=="p" || key=="precision")
                    ios_.precision(atoi(value.c_str()));
                else if(key=="locale") {
                    if(!d->restore_locale) {
                        d->saved_locale=ios_.getloc();
                        d->restore_locale=true;
                    }

                    std::string encoding=std::use_facet<info>(d->saved_locale).encoding();
                    generator gen;
                    gen.categories(formatting_facet);
                    gen.octet_encoding(encoding);
                    ios_.imbue(gen.get(value));
                }

            }
        }
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
