//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_SRC_LOCALE_HPP
#define CPPCMS_SRC_LOCALE_HPP
#include <unicode/unistr.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/noncopyable.hpp>
#else // Internal Boost
#   include <cppcms_boost/noncopyable.hpp>
    namespace boost = cppcms_boost;
#endif

#include <string>
#include "locale_src_icu_util.hpp"

namespace cppcms {
namespace locale {
namespace impl {

       
    template<typename CharType,int char_size = sizeof(CharType) >
    class icu_std_converter {
    public:
        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        icu_std_converter(std::string charset);         
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
            icu::UnicodeString tmp(begin,end-begin,charset_.c_str());
            return tmp;
        }
        
        string_type std(icu::UnicodeString const &str) const
        {
            uconv cvt(charset_);
            return cvt.go(str.getBuffer(),str.length(),max_len_);
        }

        icu_std_converter(std::string charset) : charset_(charset)
        {
            uconv cvt(charset_);
            max_len_=cvt.max_char_size();
        }

        size_t cut(icu::UnicodeString const &str,char_type const *begin,char_type const *end,
                        size_t n,size_t from_u=0,size_t from_char=0) const
        {
            size_t code_points = str.countChar32(from_u,n);
            uconv cvt(charset_);
            return cvt.cut(code_points,begin+from_char,end);
        }

        struct uconv : public boost::noncopyable {
        public:
            uconv(std::string const &charset) 
            {
                utf8_ = ucnv_compareNames(charset.c_str(),"UTF8") == 0;
                UErrorCode err=U_ZERO_ERROR;
                cvt_ = ucnv_open(charset.c_str(),&err);
                if(!cvt_)
                    throw_icu_error(err);
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
                if(utf8_) {
                    size_t res = 0;
                    while( n > 0) {
                        UChar32 uc;
                        U8_NEXT_UNSAFE(begin,res,uc);
                        n--;
                    }
                    return res;
                }

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

            ~uconv()
            {
                ucnv_close(cvt_);
            }
                
        private:
            bool utf8_; 
            UConverter *cvt_;
        };

    private:
        int max_len_;
        std::string charset_;
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
        
        icu_std_converter(std::string charset) {}

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
            int32_t len=0;
            #endif

            UErrorCode code=U_ZERO_ERROR;
            u_strToUTF32(ptr,tmp.size(),&len,str.getBuffer(),str.length(),&code);

            check_and_throw_icu_error(code);

            tmp.resize(len);

            return tmp;
        }
        
        size_t cut(icu::UnicodeString const &str,char_type const *begin,char_type const *end,size_t n,
                size_t from_u=0,size_t from_c=0) const
        {
            return str.countChar32(from_u,n);
        }

        icu_std_converter(std::string charset) {}

    };
} /// impl
} //  locale
} // boost

#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
