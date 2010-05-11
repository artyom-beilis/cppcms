//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_LOCALE_SOURCE
#include <booster/locale/info.h>
#include <unicode/locid.h>
#include <unicode/ucnv.h>

#include "info_impl.h"

namespace booster {
    namespace locale {

        std::locale::id info::id;

        info::~info()
        {
        }

        info::info(std::string posix_id,size_t refs) : 
            std::locale::facet(refs)
        {
            impl_.reset(new info_impl());

            if(posix_id.empty()) {
                impl_->encoding = ucnv_getDefaultName();
            }
            else {
                impl_->locale = icu::Locale::createCanonical(posix_id.c_str());
                size_t n = posix_id.find('.');
                if(n!=std::string::npos) {
                    size_t e = posix_id.find('@',n);
                    if(e == std::string::npos)
                        impl_->encoding = posix_id.substr(n+1);
                    else
                        impl_->encoding = posix_id.substr(n+1,e-n-1);
                }
                else
                    impl_->encoding = ucnv_getDefaultName();
            }

            if(ucnv_compareNames(impl_->encoding.c_str(),"UTF8")==0)
                utf8_=true;
            else
                utf8_=false;
        }

        info::info(std::string posix_id,std::string encoding,size_t refs) : 
            std::locale::facet(refs)
        {
            impl_.reset(new info_impl());
            size_t n = posix_id.find('.');
            if(n!=std::string::npos) {
                size_t e = posix_id.find('@',n);
                if(e == std::string::npos)
                    impl_->encoding = posix_id.substr(n+1);
                else
                    impl_->encoding = posix_id.substr(n+1,e-n-1);
            }
            else
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
