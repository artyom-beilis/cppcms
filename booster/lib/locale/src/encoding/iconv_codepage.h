//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_IMPL_ICONV_CODEPAGE_HPP
#define BOOSTER_LOCALE_IMPL_ICONV_CODEPAGE_HPP

#include <booster/locale/encoding.h>
#include "../util/iconv.h"
#include <errno.h>
#include "conv.h"
#include <assert.h>
#include <vector>

namespace booster {
namespace locale {
namespace conv {
namespace impl {

namespace {

    template<typename CharType>
    char const *utf_name()
    {
        union {
            char first;
            uint16_t u16;
            uint32_t u32;
        } v;

        if(sizeof(CharType) == 1) {
            return "UTF-8";
        }
        else if(sizeof(CharType) == 2) {
            v.u16 = 1;
            if(v.first == 1) {
                return "UTF-16LE";
            }
            else {
                return "UTF-16BE";
            }
        }
        else if(sizeof(CharType) == 4) {
            v.u32 = 1;
            if(v.first == 1) {
                return "UTF-32LE";
            }
            else {
                return "UTF-32BE";
            }

        }
        else {
            return "Unknown Character Encoding";
        }
    }
} // anon-namespace

class iconverter_base {
public:
    
    iconverter_base() : 
    cvt_((iconv_t)(-1))
    {
    }

    virtual ~iconverter_base()
    {
        close();
    }

    size_t conv(char const **inbufc,size_t *inchar_left,
                char **outbuf,size_t *outchar_left)
    {
        char **inbuf = const_cast<char **>(inbufc);
        return iconv(cvt_,inbuf,inchar_left,outbuf,outchar_left);
    }

    bool open(char const *to,char const *from)
    {
        close();
        cvt_ = iconv_open(to,from);
        return cvt_ != (iconv_t)(-1);
    }

private:

    void close()
    {
        if(cvt_!=(iconv_t)(-1)) {
            iconv_close(cvt_);
            cvt_ = (iconv_t)(-1);
        }
    }
    
    iconv_t cvt_;

};

template<typename CharType>
class iconv_from_utf : public iconverter_base, public converter_from_utf<CharType>
{
public:

    typedef CharType char_type;

    virtual bool open(char const *charset,method_type how)
    {
        if(!iconverter_base::open(charset,utf_name<CharType>()))
            return false;
        method(how);
        return true;
    }
    void method(method_type how)
    {
        how_ = how;
    }

    virtual std::string convert(char_type const *ubegin,char_type const *uend)
    {
        std::string sresult;
        std::vector<char> result((uend-ubegin)+10,'\0');
        char *out_start = &result[0];

        char const *begin = reinterpret_cast<char const *>(ubegin);
        char const *end = reinterpret_cast<char const *>(uend);
        
        enum { normal , unshifting , done } state = normal;

        while(state!=done) {

            size_t in_left = end - begin;
            size_t out_left = result.size();
            
            char *out_ptr = out_start;
            size_t res = 0;
            if(in_left == 0)
                state = unshifting;

            if(state == normal) 
                res = conv(&begin,&in_left,&out_ptr,&out_left);
            else
                res = conv(0,0,&out_ptr,&out_left);

            int err = errno;
            
            sresult.append(&result[0],out_ptr - out_start);

            if(res == (size_t)(-1)) {
                if(err == EILSEQ || err == EINVAL) {
                    if(how_ == stop) {
                        throw conversion_error();
                    }
                    else if(begin != end) {
                        begin+=sizeof(char_type);
                    }
                    else {
                        break;
                    }
                }
                else if (err==E2BIG) {
                    continue;
                }
                else {
                    break;
                }
            }
            if(state == unshifting)
                state = done;
        }
        return sresult;
    }

private:
    method_type how_;

};

class iconv_between: public iconv_from_utf<char>, public converter_between
{
public:
    virtual bool open(char const *to_charset,char const *from_charset,method_type how)
    {
        if(!iconverter_base::open(to_charset,from_charset))
            return false;
        method(how);
        return true;
    }
    virtual std::string convert(char const *begin,char const *end)
    {
        return iconv_from_utf<char>::convert(begin,end);
    }
};


template<typename CharType>
class iconv_to_utf : public iconverter_base, public converter_to_utf<CharType>
{
public:

    typedef CharType char_type;
    typedef std::basic_string<char_type> string_type;

    virtual bool open(char const *charset,method_type how)
    {
        if(!iconverter_base::open(utf_name<CharType>(),charset))
            return false;
        how_ = how;
        return true;
    }

    virtual string_type convert(char const *begin,char const *end)
    {
        string_type sresult;
        std::vector<char_type> result((end-begin)+10,char_type());
        char *out_start = reinterpret_cast<char *>(&result[0]);
        
        enum { normal , unshifting , done } state = normal;

        while(state!=done) {

            size_t in_left = end - begin;
            size_t out_left = result.size() * sizeof(char_type);
            
            char *out_ptr = out_start;

            size_t res = 0;
            if(in_left == 0)
                state = unshifting;

            if(state == normal) 
                res = conv(&begin,&in_left,&out_ptr,&out_left);
            else
                res = conv(0,0,&out_ptr,&out_left);

            int err = errno;

            sresult.append(&result[0],(out_ptr - out_start)/sizeof(char_type));

            if(res == (size_t)(-1)) {
                if(err == EILSEQ || err == EINVAL) {
                    if(how_ == stop) {
                        throw conversion_error();
                    }
                    else if(begin != end) {
                        begin++;
                    }
                    else {
                        break;
                    }
                }
                else if (err==E2BIG) {
                    continue;
                }
                else {
                    break;
                }
            }
            if(state == unshifting)
                state = done;
        }
        return sresult;
    }

private:
    method_type how_;
};




} // impl
} // conv
} // locale 
} // boost




#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
