//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOSTER_LOCALE_IMPL_WCONV_CODEPAGE_HPP
#define BOOSTER_LOCALE_IMPL_WCONV_CODEPAGE_HPP


#include <booster/locale/encoding.h>
#include <algorithm>
#include "conv.h"

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>
#include <vector>


namespace booster {
namespace locale {
namespace conv {
namespace impl {
    
    struct windows_encoding {
        char const *name;
        unsigned codepage;
    };

    bool operator<(windows_encoding const &l,windows_encoding const &r)
    {
        return strcmp(l.name,r.name) < 0;
    }

    windows_encoding all_windows_encodings[] = {
        { "big5",       950 },
        { "cp1250",     1250 },
        { "cp1251",     1251 },
        { "cp1252",     1252 },
        { "cp1253",     1253 },
        { "cp1254",     1254 },
        { "cp1255",     1255 },
        { "cp1256",     1256 },
        { "cp1257",     1257 },
        { "cp874",      874 },
        { "cp932",      932 },
        { "eucjp",      20932 },
        { "euckr",      51949 },
        { "gb18030",    54936 },
        { "gb2312",     936 },
        { "iso2022jp",  50220 },
        { "iso2022kr",  50225 },
        { "iso88591",   28591 },
        { "iso885913",  28603 },
        { "iso885915",  28605 },
        { "iso88592",   28592 },
        { "iso88593",   28593 },
        { "iso88594",   28594 },
        { "iso88595",   28595 },
        { "iso88596",   28596 },
        { "iso88597",   28597 },
        { "iso88598",   28598 },
        { "iso88599",   28599 },
        { "koi8r",      20866 },
        { "koi8u",      21866 },
        { "shiftjis",   932 },
        { "sjis",       932 },
        { "usascii",    20127 },
        { "utf8",       65001 },
        { "windows1250",        1250 },
        { "windows1251",        1251 },
        { "windows1252",        1252 },
        { "windows1253",        1253 },
        { "windows1254",        1254 },
        { "windows1255",        1255 },
        { "windows1256",        1256 },
        { "windows1257",        1257 },
        { "windows874",         874 },
        { "windows932",         932 },
    };

    size_t remove_substitutions(std::vector<wchar_t> &v)
    {
        if(std::find(v.begin(),v.end(),wchar_t(0xFFFD)) == v.end()) {
            return v.size();
        }
        std::vector<wchar_t> v2;
        v2.reserve(v.size());
        for(unsigned i=0;i<v.size();i++) {
            if(v[i]!=0xFFFD)
                v2.push_back(v[i]);
        }
        v.swap(v2);
        return v.size();
    }

    size_t remove_substitutions(std::vector<char> &v)
    {
        if(std::find(v.begin(),v.end(),0) == v.end()) {
            return v.size();
        }
        std::vector<char> v2;
        v2.reserve(v.size());
        for(unsigned i=0;i<v.size();i++) {
            if(v[i]!=0)
                v2.push_back(v[i]);
        }
        v.swap(v2);
        return v.size();
    }

    
    void multibyte_to_wide(int codepage,char const *begin,char const *end,bool do_skip,std::vector<wchar_t> &buf)
    {
        if(begin==end)
            return;
        DWORD flags = do_skip ? 0 : MB_ERR_INVALID_CHARS;
        if(50220 <= codepage && codepage <= 50229)
            flags = 0;
        
        int n = MultiByteToWideChar(codepage,flags,begin,end-begin,0,0);
        if(n == 0)
            throw conversion_error();
        buf.resize(n,0);
        if(MultiByteToWideChar(codepage,flags,begin,end-begin,&buf.front(),buf.size())==0)
            throw conversion_error();
        if(do_skip)
            remove_substitutions(buf);
    }

    void wide_to_multibyte_non_zero(int codepage,wchar_t const *begin,wchar_t const *end,bool do_skip,std::vector<char> &buf)
    {
        if(begin==end)
            return;
        BOOL substitute = FALSE;
        BOOL *substitute_ptr = codepage == 65001 || codepage == 65000 ? 0 : &substitute;
        char subst_char = 0;
        char *subst_char_ptr = codepage == 65001 || codepage == 65000 ? 0 : &subst_char;
        
        int n = WideCharToMultiByte(codepage,0,begin,end-begin,0,0,subst_char_ptr,substitute_ptr);
        buf.resize(n);
        
        if(WideCharToMultiByte(codepage,0,begin,end-begin,&buf[0],n,subst_char_ptr,substitute_ptr)==0)
            throw conversion_error();
        if(substitute) {
            if(do_skip) 
                remove_substitutions(buf);
            else 
                throw conversion_error();
        }
    }
    
    void wide_to_multibyte(int codepage,wchar_t const *begin,wchar_t const *end,bool do_skip,std::vector<char> &buf)
    {
        if(begin==end)
            return;
        buf.reserve(end-begin);
        wchar_t const *e = std::find(begin,end,L'\0');
        wchar_t const *b = begin;
        for(;;) {
            std::vector<char> tmp;
            wide_to_multibyte_non_zero(codepage,b,e,do_skip,tmp);
            size_t osize = buf.size();
            buf.resize(osize+tmp.size());
            std::copy(tmp.begin(),tmp.end(),buf.begin()+osize);
            if(e!=end) {
                buf.push_back('\0');
                b=e+1;
                e=std::find(b,end,L'0');
            }
            else 
                break;
        }
    }

    
    int encoding_to_windows_codepage(char const *ccharset)
    {
        std::string charset = normalize_encoding(ccharset);
        windows_encoding ref;
        ref.name = charset.c_str();
        size_t n = sizeof(all_windows_encodings)/sizeof(all_windows_encodings[0]);
        windows_encoding *begin = all_windows_encodings;
        windows_encoding *end = all_windows_encodings + n;
        windows_encoding *ptr = std::lower_bound(begin,end,ref);
        if(ptr!=end && strcmp(ptr->name,charset.c_str())==0) {
            return ptr->codepage;
        }
        return -1;
        
    }

    bool validate_utf16(uint16_t const *str,unsigned len)
    {
        for(unsigned i=0;i<len;i++) {
           if(0xD800 <= str[i] && str[i]<= 0xDBFF) {
               i++;
               if(i>=len)
                   return false;
                if(0xDC00 <= str[i] && str[i]<=0xDFFF)
                    continue;
                return false;
           }
           else if(0xDC00 <= str[i] && str[i]<=0xDFFF)
               return false;
        }
        return true;
    }

    class wconv_between : public converter_between {
    public:
        wconv_between() : 
            how_(skip),
            to_code_page_ (-1),
            from_code_page_ ( -1)
        {
        }
        bool open(char const *to_charset,char const *from_charset,method_type how)
        {
            how_ = how;
            to_code_page_ = encoding_to_windows_codepage(to_charset);
            from_code_page_ = encoding_to_windows_codepage(from_charset);
            if(to_code_page_ == -1 || from_code_page_ == -1)
                return false;
            return true;
        }
        virtual std::string convert(char const *begin,char const *end)
        {
            std::string res;
            std::vector<wchar_t> tmp;
            multibyte_to_wide(from_code_page_,begin,end,how_ == skip,tmp);
            if(tmp.empty())
                return res;
            std::vector<char> ctmp;
            wide_to_multibyte(to_code_page_,&tmp.front(),&tmp.front()+tmp.size(),how_ == skip,ctmp);
            if(ctmp.empty())
                return res;
            res.assign(&ctmp.front(),ctmp.size());
            return res;
        }
    private:
        method_type how_;
        int to_code_page_;
        int from_code_page_;
    };
    
    template<typename CharType,int size = sizeof(CharType) >
    class wconv_to_utf;

    template<typename CharType,int size = sizeof(CharType) >
    class wconv_from_utf;

    template<>
    class wconv_to_utf<char,1> : public  converter_to_utf<char> , public wconv_between {
    public:
        virtual bool open(char const *cs,method_type how) 
        {
            return wconv_between::open("UTF-8",cs,how);
        }
        virtual std::string convert(char const *begin,char const *end)
        {
            return wconv_between::convert(begin,end);
        }
    };
    
    template<>
    class wconv_from_utf<char,1> : public  converter_from_utf<char> , public wconv_between {
    public:
        virtual bool open(char const *cs,method_type how) 
        {
            return wconv_between::open(cs,"UTF-8",how);
        }
        virtual std::string convert(char const *begin,char const *end)
        {
            return wconv_between::convert(begin,end);
        }
    };
    
    template<typename CharType>
    class wconv_to_utf<CharType,2> : public converter_to_utf<CharType> {
    public:
        typedef CharType char_type;

        typedef std::basic_string<char_type> string_type;

        wconv_to_utf() : 
            how_(skip),
            code_page_(-1)
        {
        }

        virtual bool open(char const *charset,method_type how)
        {
            how_ = how;
            code_page_ = encoding_to_windows_codepage(charset);
            return code_page_ != -1;
        }

        virtual string_type convert(char const *begin,char const *end) 
        {
            std::vector<wchar_t> tmp;
            multibyte_to_wide(code_page_,begin,end,how_ == skip,tmp);
            string_type res;
            if(!tmp.empty())
                res.assign(reinterpret_cast<char_type *>(&tmp.front()),tmp.size());
            return res;
        }

    private:
        method_type how_;
        int code_page_;
    };
  
    template<typename CharType>
    class wconv_from_utf<CharType,2> : public converter_from_utf<CharType> {
    public:
        typedef CharType char_type;

        typedef std::basic_string<char_type> string_type;

        wconv_from_utf() : 
            how_(skip),
            code_page_(-1)
        {
        }

        virtual bool open(char const *charset,method_type how)
        {
            how_ = how;
            code_page_ = encoding_to_windows_codepage(charset);
            return code_page_ != -1;
        }

        virtual std::string convert(CharType const *begin,CharType const *end) 
        {
            if(begin==end)
                return std::string();
            if(how_ == stop && !validate_utf16(reinterpret_cast<uint16_t const *>(begin),end-begin)) {
                throw conversion_error();
            }
            std::vector<char> ctmp;
			wchar_t const *wbegin = reinterpret_cast<wchar_t const *>(begin);
			wchar_t const *wend = reinterpret_cast<wchar_t const *>(end);
            wide_to_multibyte(code_page_,wbegin,wend,how_ == skip,ctmp);
            std::string res;
            if(ctmp.empty())
                return res;
            res.assign(&ctmp.front(),ctmp.size());
            return res;
        }

    private:
        method_type how_;
        int code_page_;
    };



    template<typename CharType>
    class wconv_to_utf<CharType,4> : public converter_to_utf<CharType> {
    public:
        typedef CharType char_type;

        typedef std::basic_string<char_type> string_type;

        wconv_to_utf() : 
            how_(skip),
            code_page_(-1)
        {
        }

        virtual bool open(char const *charset,method_type how)
        {
            how_ = how;
            code_page_ = encoding_to_windows_codepage(charset);
            return code_page_ != -1;
        }

        virtual string_type convert(char const *begin,char const *end) 
        {
            std::vector<wchar_t> buf;
            multibyte_to_wide(code_page_,begin,end,how_ == skip,buf);
            remove_substitutions(buf);

            size_t n=buf.size();
            string_type res;
            res.reserve(n);
            for(unsigned i=0;i<n;i++) {
                wchar_t cur = buf[i];
                if(0xD800 <= cur && cur<= 0xDBFF) {
                    i++;
                    if(i>=n)
                        throw conversion_error();
                    if(0xDC00 <= buf[i] && buf[i]<=0xDFFF) {
                        uint32_t w1 = cur;
                        uint32_t w2 = buf[i];
                        uint32_t norm = ((uint32_t(w1 & 0x3FF) << 10) | (w2 & 0x3FF)) + 0x10000;
                        res+=char_type(norm);
                    }
                    else 
                        throw conversion_error();
                }
                else if(0xDC00 <= cur && cur<=0xDFFF)
                    throw conversion_error();
                else
                    res+=char_type(cur);
            }
            return res;
        }
    private:
        method_type how_;
        int code_page_;
    };
  
    template<typename CharType>
    class wconv_from_utf<CharType,4> : public converter_from_utf<CharType> {
    public:
        typedef CharType char_type;

        typedef std::basic_string<char_type> string_type;

        wconv_from_utf() : 
            how_(skip),
            code_page_(-1)
        {
        }

        virtual bool open(char const *charset,method_type how)
        {
            how_ = how;
            code_page_ = encoding_to_windows_codepage(charset);
            return code_page_ != -1;
        }

        virtual std::string convert(CharType const *begin,CharType const *end) 
        {
            std::wstring tmp;
            tmp.reserve(end-begin);
            while(begin!=end) {
                uint32_t cur = *begin++;
                if(cur > 0x10FFFF  || (0xD800 <=cur && cur <=0xDFFF)) {
                    if(how_ == skip)
                        continue;
                    else
                        throw conversion_error();
                }
                if(cur > 0xFFFF) {
                    uint32_t u = cur - 0x10000;
                    wchar_t first  = 0xD800 | (u>>10);
                    wchar_t second = 0xDC00 | (u & 0x3FF);
                    tmp+=first;
                    tmp+=second;
                }
                else {
                    tmp+=wchar_t(cur);
                }
            }

            std::vector<char> ctmp;
            wide_to_multibyte(code_page_,tmp.c_str(),tmp.c_str()+tmp.size(),how_ == skip,ctmp);
            std::string res;
            if(ctmp.empty())
                return res;
            res.assign(&ctmp.front(),ctmp.size());
            return res;

        }

    private:
        method_type how_;
        int code_page_;
    };





} // impl
} // conv
} // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
