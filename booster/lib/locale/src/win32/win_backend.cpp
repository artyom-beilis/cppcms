//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/locale/localization_backend.h>
#include <booster/locale/gnu_gettext.h>
#include <booster/locale/info.h>
#include "all_generator.h"
#include "win_backend.h"
#include <booster/locale/util.h>
#include "../util/locale_data.h"
#include "api.h"

namespace booster {
namespace locale {
namespace impl_win { 
    
    class posix_localization_backend : public localization_backend {
    public:
        posix_localization_backend() : 
            invalid_(true)
        {
        }
        posix_localization_backend(posix_localization_backend const &other) : 
            paths_(other.paths_),
            domains_(other.domains_),
            locale_id_(other.locale_id_),
            invalid_(true)
        {
        }
        virtual posix_localization_backend *clone() const
        {
            return new posix_localization_backend(*this);
        }

        void set_option(std::string const &name,std::string const &value) 
        {
            invalid_ = true;
            if(name=="locale")
                locale_id_ = value;
            else if(name=="message_path")
                paths_.push_back(value);
            else if(name=="message_application")
                domains_.push_back(value);

        }
        void clear_options()
        {
            invalid_ = true;
            locale_id_.clear();
            paths_.clear();
            domains_.clear();
        }

        void prepare_data()
        {
            if(!invalid_)
                return;
            invalid_ = false;
            if(locale_id_.empty()) {
                real_id_ = util::get_system_locale(true);
                lc_ = winlocale(real_id_);
            }
            else {
                lc_=winlocale(locale_id_);
                real_id_ = locale_id_;
            }
            util::locale_data d;
            d.parse(real_id_);
            if(!d.utf8) {
                lc_ = winlocale(); 
                // Make it C as non-UTF8 locales are not supported
            }
        }
        
        virtual std::locale install(std::locale const &base,
                                    locale_category_type category,
                                    character_facet_type type = nochar_facet)
        {
            prepare_data();

            switch(category) {
            case convert_facet:
                return create_convert(base,lc_,type);
            case collation_facet:
                return create_collate(base,lc_,type);
            case formatting_facet:
                return create_formatting(base,lc_,type);
            case parsing_facet:
                return create_parsing(base,lc_,type);
            case message_facet:
                {
                    gnu_gettext::messages_info minf;
                    std::locale tmp=util::create_info(std::locale::classic(),real_id_);
                    booster::locale::info const &inf=std::use_facet<booster::locale::info>(tmp);
                    minf.language = inf.language();
                    minf.country = inf.country();
                    minf.variant = inf.variant();
                    minf.encoding = inf.encoding();
                    minf.domains = domains_;
                    minf.paths = paths_;
                    switch(type) {
                    case char_facet:
                        return std::locale(base,gnu_gettext::create_messages_facet<char>(minf));
                    case wchar_t_facet:
                        return std::locale(base,gnu_gettext::create_messages_facet<wchar_t>(minf));
                    default:
                        return base;
                    }
                }
            case information_facet:
                return util::create_info(base,real_id_);
            case codepage_facet:
                return util::create_codecvt(base,util::create_utf8_converter(),type);
            default:
                return base;
            }
        }

    private:

        std::vector<std::string> paths_;
        std::vector<std::string> domains_;
        std::string locale_id_;
        std::string real_id_;

        bool invalid_;
        winlocale lc_;
    };
    
    localization_backend *create_localization_backend()
    {
        return new posix_localization_backend();
    }

}  // impl win
}  // locale
}  // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
