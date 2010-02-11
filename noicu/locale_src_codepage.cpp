//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_codepage.h"

#include <unicode/ucnv.h>
#include <unicode/ucnv_err.h>

#include "locale_src_icu_util.hpp"

namespace cppcms {
namespace locale {
namespace details {
    struct converter::data {
        data() : cvt(0) {}
        ~data() { if(cvt) ucnv_close(cvt); }
        UConverter *cvt; 
    };
    converter::converter(std::string const &encoding)
    {
        is_utf8_ = ucnv_compareNames(encoding.c_str(),"UTF8") == 0;
        if(is_utf8_)
            max_len_=4;
        else {
            d.reset(new data());
            UErrorCode err=U_ZERO_ERROR;
            d->cvt = ucnv_open(encoding.c_str(),&err);
            if(!d->cvt)
                impl::throw_icu_error(err);
            
            err=U_ZERO_ERROR;

            ucnv_setFromUCallBack(d->cvt,UCNV_FROM_U_CALLBACK_STOP,0,0,0,&err);
            impl::check_and_throw_icu_error(err);
            
            err=U_ZERO_ERROR;
            ucnv_setToUCallBack(d->cvt,UCNV_TO_U_CALLBACK_STOP,0,0,0,&err);
            impl::check_and_throw_icu_error(err);
            
            max_len_ = ucnv_getMaxCharSize(d->cvt);
        }
    }
    converter::~converter()
    {
    }
    uint32_t converter::from_charset(char const *&begin,char const *end)
    {
        UErrorCode err=U_ZERO_ERROR;
        UChar32 c=ucnv_getNextUChar(d->cvt,&begin,end,&err);
        if(err==U_INDEX_OUTOFBOUNDS_ERROR)
            return incomplete;
        if(U_FAILURE(err))
            return illegal;
        return c;
    }
    uint32_t converter::to_charset(uint32_t u,char *begin,char const *end)
    {
        UChar code_point[2]={0};
        int len;
        if(u<=0xFFFF) {
            if(0xD800 <=u && u<= 0xDFFF) // No surragates
                return illegal;
            code_point[0]=u;
            len=1;
        }
        else {
            u-=0x10000;
            code_point[0]=0xD800 | (u>>10);
            code_point[1]=0xDC00 | (u & 0x3FF);
            len=2;
        }
        UErrorCode err=U_ZERO_ERROR;
        int olen = ucnv_fromUChars(d->cvt,begin,end-begin,code_point,len,&err);
        if(U_FAILURE(err))
            return illegal;
        if(olen > end-begin)
            return incomplete;
        return olen;
    }


} // details
} // locale 
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
