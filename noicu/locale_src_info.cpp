//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_info.h"
#include "locale_src_info_impl.hpp"

namespace cppcms {
    namespace locale {
        
        std::locale::id info::id;

        info::~info()
        {
        }

        info::info(std::string posix_id,std::string encoding,size_t refs) : 
            std::locale::facet(refs)
        {
            impl_.reset(new info_impl());

            static boost::regex locm("^([a-zA-Z]+)([\-_ ]([a-zA-Z]+))?(\.([a-zA-Z_-0-9]+))?(@([a-zA-Z_-0-9]))?$");

        }

        info::info(std::string posix_id,std::string encoding,size_t refs) : 
            std::locale::facet(refs)
        {
            impl_.reset(new info_impl());
            impl_->encoding = encoding;

            if(!posix_id.empty()) {
                impl_->locale = icu::Locale::createCanonical(posix_id.c_str());
            }
            
            if(ucnv_compareNames(impl_->encoding.c_str(),"UTF8")==0)
                utf8_=true;
            else
                utf8_=false;
        }

        std::string info::encoding() const
        {
            return impl()->encoding;
        }
        std::string info::language() const
        {
            return impl()->locale.getLanguage();
        }
        std::string info::country() const
        {
            return impl()->locale.getCountry();
        }
        std::string info::variant() const
        {
            return impl()->locale.getVariant();
        }
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
