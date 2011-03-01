//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_SRC_LOCALE_HPP
#define BOOSTER_SRC_LOCALE_HPP
#include <unicode/unistr.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>

#include <booster/locale/encoding.h>

#include <string>
#include "icu_util.h"

namespace booster {
namespace locale {
namespace impl_icu {

    typedef enum {
        cvt_skip,
        cvt_stop
    } cpcvt_type;

       
    template<typename CharType,int char_size = sizeof(CharType) >
    class icu_std_converter {
    public:
        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        icu_std_converter(std::string charset,cpcvt_type cv=cvt_skip);         
        icu::UnicodeString icu(char_type const *begin,char_type const *end) const;
        string_type std(icu::UnicodeString const &str) const;
        size_t cut(icu::UnicodeString const &str,char_type const *begin,char_type const *end,size_t n,size_t from_u=0,size_t from_c=0) const;
    };

    template<typename CharType>
    class icu_std_converter<CharType,1> {
    public:
        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        
        icu::UnicodeString icu(char_type const *vb,char_type const *ve) const
        {
            char const *begin=reinterpret_cast<char const *>(vb);
            char const *end=reinterpret_cast<char const *>(ve);
            uconv cvt(charset_,cvt_type_);
            UErrorCode err=U_ZERO_ERROR;
            icu::UnicodeString tmp(begin,end-begin,cvt.cvt(),err);
            check_and_throw_icu_error(err);
            return tmp;
        }
        
        string_type std(icu::UnicodeString const &str) const
        {
            uconv cvt(charset_,cvt_type_);
            return cvt.go(str.getBuffer(),str.length(),max_len_);
        }

        icu_std_converter(std::string charset,cpcvt_type cvt_type = cvt_skip) : 
            charset_(charset),
            cvt_type_(cvt_type)
        {
            uconv cvt(charset_,cvt_type);
            max_len_=cvt.max_char_size();
        }

        size_t cut(icu::UnicodeString const &str,char_type const *begin,char_type const *end,
                        size_t n,size_t from_u=0,size_t from_char=0) const
        {
            size_t code_points = str.countChar32(from_u,n);
            uconv cvt(charset_,cvt_type_);
            return cvt.cut(code_points,begin+from_char,end);
        }

        struct uconv  {
            uconv(uconv const &other);
            void operator=(uconv const &other);
        public:
            uconv(std::string const &charset,cpcvt_type cvt_type=cvt_skip) 
            {
                UErrorCode err=U_ZERO_ERROR;
                cvt_ = ucnv_open(charset.c_str(),&err);
                if(!cvt_ || U_FAILURE(err)) {
                    if(cvt_)
                        ucnv_close(cvt_);
                    throw conv::invalid_charset_error(charset);
                }
                
                try {
                    if(cvt_type==cvt_skip) {
                        ucnv_setFromUCallBack(cvt_,UCNV_FROM_U_CALLBACK_SKIP,0,0,0,&err);
                        check_and_throw_icu_error(err);
                
                        err=U_ZERO_ERROR;
                        ucnv_setToUCallBack(cvt_,UCNV_TO_U_CALLBACK_SKIP,0,0,0,&err);
                        check_and_throw_icu_error(err);
                    }
                    else {
                        ucnv_setFromUCallBack(cvt_,UCNV_FROM_U_CALLBACK_STOP,0,0,0,&err);
                        check_and_throw_icu_error(err);
                
                        err=U_ZERO_ERROR;
                        ucnv_setToUCallBack(cvt_,UCNV_TO_U_CALLBACK_STOP,0,0,0,&err);
                        check_and_throw_icu_error(err);
                    }
                }
                catch(...) { ucnv_close(cvt_) ; throw; }
            }

            int max_char_size()
            {
                return ucnv_getMaxCharSize(cvt_);
            }

            string_type go(UChar const *buf,int length,int max_size)
            {
                string_type res;
                res.resize(UCNV_GET_MAX_BYTES_FOR_STRING(length,max_size));
                char *ptr=reinterpret_cast<char *>(&res[0]);
                UErrorCode err=U_ZERO_ERROR;
                int n = ucnv_fromUChars(cvt_,ptr,res.size(),buf,length,&err);
                check_and_throw_icu_error(err);
                res.resize(n);
                return res;
            }

            size_t cut(size_t n,char_type const *begin,char_type const *end)
            {
                char_type const *saved = begin;
                while(n > 0 && begin < end) {
                    UErrorCode err=U_ZERO_ERROR;
                    ucnv_getNextUChar(cvt_,&begin,end,&err);
                    if(U_FAILURE(err))
                        return 0;
                    n--;
                }
                return begin - saved;
            }

            UConverter *cvt() { return cvt_; }

            ~uconv()
            {
                ucnv_close(cvt_);
            }
                
        private:
            UConverter *cvt_;
        };

    private:
        int max_len_;
        std::string charset_;
        cpcvt_type cvt_type_;
    };
   
    template<typename CharType>
    class icu_std_converter<CharType,2> {
    public:
        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        
        icu::UnicodeString icu(char_type const *vb,char_type const *ve) const
        {
            UChar const *begin=reinterpret_cast<UChar const *>(vb);
            UChar const *end=reinterpret_cast<UChar const *>(ve);
            icu::UnicodeString tmp(begin,end-begin);
            return tmp;

        }

        string_type std(icu::UnicodeString const &str) const
        {
            char_type const *ptr=reinterpret_cast<char_type const *>(str.getBuffer());
            return string_type(ptr,str.length());
        }
        size_t cut(icu::UnicodeString const &str,char_type const *begin,char_type const *end,size_t n,
                        size_t from_u=0,size_t from_c=0) const
        {
            return n;
        }
        
        icu_std_converter(std::string charset,cpcvt_type unused=cvt_skip) {}

    };
    
    template<typename CharType>
    class icu_std_converter<CharType,4> {
    public:

        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        
        icu::UnicodeString icu(char_type const *vb,char_type const *ve) const
        {
            UChar32 const *begin=reinterpret_cast<UChar32 const *>(vb);
            UChar32 const *end  =reinterpret_cast<UChar32 const *>(ve);

            icu::UnicodeString tmp(end-begin,0,0); // make inital capacity
            while(begin!=end)
                tmp.append(*begin++);
            return tmp;

        }

        string_type std(icu::UnicodeString const &str) const
        {
            string_type tmp;
            tmp.resize(str.length());
            UChar32 *ptr=reinterpret_cast<UChar32 *>(&tmp[0]);

            #ifdef __SUNPRO_CC
            int len=0;
            #else
            ::int32_t len=0;
            #endif

            UErrorCode code=U_ZERO_ERROR;
            u_strToUTF32(ptr,tmp.size(),&len,str.getBuffer(),str.length(),&code);

            check_and_throw_icu_error(code);

            tmp.resize(len);

            return tmp;
        }
        
        size_t cut(icu::UnicodeString const &str,char_type const * /*begin*/,char_type const * /*end*/,size_t n,
                size_t from_u=0,size_t /*from_c*/=0) const
        {
            return str.countChar32(from_u,n);
        }

        icu_std_converter(std::string /*charset*/,cpcvt_type /*unused*/=cvt_skip) {}

    };
} /// impl_icu
} //  locale
} // boost

#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
