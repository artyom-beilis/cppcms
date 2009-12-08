//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_boundary.h"
#include "locale_info.h"
#include <unicode/uversion.h>
#if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 306
#include <unicode/utext.h>
#endif
#include <unicode/brkiter.h>
#include <unicode/rbbi.h>

#include "locale_src_icu_util.hpp"
#include "locale_src_info_impl.hpp"
#include "locale_src_uconv.hpp"

namespace cppcms {
namespace locale {
namespace boundary {
namespace impl {

using namespace cppcms::locale::impl;

index_type map_direct(boundary_type t,icu::BreakIterator *it,int reserve)
{
    index_type indx;
    indx.reserve(reserve);
    icu::RuleBasedBreakIterator *rbbi=dynamic_cast<icu::RuleBasedBreakIterator *>(it);
    
    indx.push_back(break_info());
    it->first();
    int pos=0;
    while((pos=it->next())!=icu::BreakIterator::DONE) {
        indx.push_back(break_info(pos));
        if(rbbi && (t==word || t==line))
        {
			//
			// There is a collapse for MSVC: int32_t defined by both boost::cstdint and icu...
			// So need to pick one ;(
			//
            ::int32_t buf[16]={0};
            UErrorCode err=U_ZERO_ERROR;
            int n = rbbi->getRuleStatusVec(buf,16,err);
            if(n > 16)
                n=16;
            for(int i=0;i<n;i++) {
                if(t==word) {
                    if(UBRK_WORD_NUMBER<=buf[i] && buf[i]<UBRK_WORD_NUMBER_LIMIT)
                        indx.back().prev |= number;
                    else if(UBRK_WORD_LETTER<=buf[i] && buf[i]<UBRK_WORD_LETTER_LIMIT)
                        indx.back().prev |= letter;
                    else if(UBRK_WORD_KANA<=buf[i] && buf[i]<UBRK_WORD_KANA_LIMIT)
                        indx.back().prev |= kana;
                    else if(UBRK_WORD_IDEO<=buf[i] && buf[i]<UBRK_WORD_IDEO_LIMIT)
                        indx.back().prev |= ideo;
                }
                else {
                    if(UBRK_LINE_SOFT<=buf[i] && buf[i]<UBRK_LINE_SOFT_LIMIT)
                        indx.back().brk |= soft;
                    else if(UBRK_LINE_HARD<=buf[i] && buf[i]<UBRK_LINE_HARD_LIMIT)
                        indx.back().brk |= hard;
                }
            }
        }
    }
    if(rbbi && t==word) {
        for(unsigned i=0;i<indx.size()-1;i++)
            indx[i].next=indx[i+1].prev;
    }
    return indx;
}

std::auto_ptr<icu::BreakIterator> get_iterator(boundary_type t,std::locale const &l)
{
    icu::Locale const &loc=std::use_facet<info>(l).impl()->locale;
    UErrorCode err=U_ZERO_ERROR;
    std::auto_ptr<icu::BreakIterator> bi;
    switch(t) {
    case character:
        bi.reset(icu::BreakIterator::createCharacterInstance(loc,err));
        break;
    case word:
        bi.reset(icu::BreakIterator::createWordInstance(loc,err));
        break;
    case sentence:
        bi.reset(icu::BreakIterator::createSentenceInstance(loc,err));
        break;
    case line:
        bi.reset(icu::BreakIterator::createLineInstance(loc,err));
        break;
    default:
        throw std::runtime_error("Invalid iteration type");
    }
    check_and_throw_icu_error(err);
    if(!bi.get())
        throw std::runtime_error("Failed to create break iterator");
    return bi;
}


template<typename CharType>
index_type do_map(boundary_type t,CharType const *begin,CharType const *end,std::locale const &loc)
{
    index_type indx;
    info const &inf=std::use_facet<info>(loc);
    std::auto_ptr<icu::BreakIterator> bi(get_iterator(t,loc));
   
    #if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 306
    UErrorCode err=U_ZERO_ERROR;
    if(sizeof(CharType) == 2 || (sizeof(CharType)==1 && inf.utf8()))
    {
        UText *ut=0;
        try {
            if(sizeof(CharType)==1)
                ut=utext_openUTF8(0,reinterpret_cast<char const *>(begin),end-begin,&err);
            else // sizeof(CharType)==2
                ut=utext_openUChars(0,reinterpret_cast<UChar const *>(begin),end-begin,&err);

            check_and_throw_icu_error(err);
            err=U_ZERO_ERROR;
            if(!ut) throw std::runtime_error("Failed to create UText");
            bi->setText(ut,err);
            check_and_throw_icu_error(err);
            index_type res=map_direct(t,bi.get(),end-begin);
            indx.swap(res);
        }
        catch(...) {
            if(ut)
                utext_close(ut);
            throw;
        }
        if(ut) utext_close(ut);
    }
    else 
#endif
    {
        impl::icu_std_converter<CharType> cvt(inf.encoding());
        icu::UnicodeString str=cvt.icu(begin,end);
        bi->setText(str);
        index_type indirect = map_direct(t,bi.get(),str.length());
        indx=indirect;
        for(unsigned i=1;i<indirect.size();i++) {
            unsigned offset_inderect=indirect[i-1].offset;
            unsigned diff = indirect[i].offset - offset_inderect;
            unsigned offset_direct=indx[i-1].offset;
            indx[i].offset=offset_direct + cvt.cut(str,begin,end,diff,offset_inderect,offset_direct);
        }
    }
    return indx;
} // do_map


template<>
CPPCMS_API index_type 
map(boundary_type t,char const *begin,char const *end,std::locale const &loc)
{
    return do_map(t,begin,end,loc);
}

#ifndef CPPCMS_NO_STD_WSTRING
template<>
CPPCMS_API index_type 
map(boundary_type t,wchar_t const *begin,wchar_t const *end,std::locale const &loc)
{
    return do_map(t,begin,end,loc);
}
#endif

#ifdef CPPCMS_HAS_CHAR16_T
template<>
CPPCMS_API index_type 
map(boundary_type t,char16_t const *begin,char16_t const *end,std::locale const &loc)
{
    return do_map(t,begin,end,loc);
}
#endif

#ifdef CPPCMS_HAS_CHAR32_T
template<>
CPPCMS_API index_type 
map(boundary_type t,char32_t const *begin,char32_t const *end,std::locale const &loc)
{
    return do_map(t,begin,end,loc);
}
#endif
} // impl
} // boundary
} // locale
} // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
