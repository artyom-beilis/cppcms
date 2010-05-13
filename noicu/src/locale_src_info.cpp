//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include <cppcms/locale_info.h>
#include <cppcms/locale_conversion.h>
#include "locale_src_info_impl.hpp"

namespace cppcms {
    namespace locale {
        
        std::locale::id info::id;

        info::~info()
        {
        }

        info::info(std::string lang,std::string country,std::string variant,std::string encoding,size_t refs) : 
            std::locale::facet(refs),
            impl_(new info_impl())
        {
            impl_->language = lang;
            impl_->country = country;
            impl_->variant = variant;
            impl_->encoding = encoding;
            encoding = to_lower(encoding,std::locale::classic());
            if(encoding == "utf-8" || encoding=="utf8" || encoding == "cp65001" || encoding=="65001")
                utf8_ = true;
            else
                utf8_ = false;

        }

        std::string info::encoding() const
        {
            return impl_->encoding;
        }
        std::string info::language() const
        {
            return impl_->language;
        }
        std::string info::country() const
        {
            return impl_->country;
        }
        std::string info::variant() const
        {
            return impl_->variant;
        }
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
