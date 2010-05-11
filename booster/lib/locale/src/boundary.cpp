//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_LOCALE_SOURCE
#include <booster/locale/boundary.h>
#include <booster/locale/info.h>
#include <unicode/uversion.h>
#if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 306
#include <unicode/utext.h>
#endif
#include <unicode/brkiter.h>
#include <unicode/rbbi.h>

#include "icu_util.h"
#include "info_impl.h"
#include "uconv.h"

namespace booster {
namespace locale {
namespace boundary {
namespace impl {

using namespace booster::locale::impl;

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
        /// Character does not have any specific break types
        if(t!=character && rbbi) {
            //
            // There is a collapse for MSVC: int32_t defined by both boost::cstdint and icu...
            // So need to pick one ;(
            //
            std::vector< ::int32_t> buffer;
            ::int32_t membuf[8]={0}; // try not to use memory allocation if possible
            ::int32_t *buf=membuf;

            UErrorCode err=U_ZERO_ERROR;
            int n = rbbi->getRuleStatusVec(buf,8,err);
            
            if(err == U_BUFFER_OVERFLOW_ERROR) {
                buf=&buffer.front();
                buffer.resize(n,0);
                n = rbbi->getRuleStatusVec(buf,buffer.size(),err);
            }

            impl::check_and_throw_icu_error(err);

            for(int i=0;i<n;i++) {
                switch(t) {
                case word:
                    if(UBRK_WORD_NONE<=buf[i] && buf[i]<UBRK_WORD_NONE_LIMIT)
                        indx.back().mark |= word_none;
                    else if(UBRK_WORD_NUMBER<=buf[i] && buf[i]<UBRK_WORD_NUMBER_LIMIT)
                        indx.back().mark |= word_number;
                    else if(UBRK_WORD_LETTER<=buf[i] && buf[i]<UBRK_WORD_LETTER_LIMIT)
                        indx.back().mark |= word_letter;
                    else if(UBRK_WORD_KANA<=buf[i] && buf[i]<UBRK_WORD_KANA_LIMIT)
                        indx.back().mark |= word_kana;
                    else if(UBRK_WORD_IDEO<=buf[i] && buf[i]<UBRK_WORD_IDEO_LIMIT)
                        indx.back().mark |= word_ideo;
                    break;

                case line:
                    if(UBRK_LINE_SOFT<=buf[i] && buf[i]<UBRK_LINE_SOFT_LIMIT)
                        indx.back().mark |= line_soft;
                    else if(UBRK_LINE_HARD<=buf[i] && buf[i]<UBRK_LINE_HARD_LIMIT)
                        indx.back().mark |= line_hard;
                    break;

                case sentence:
                    if(UBRK_SENTENCE_TERM<=buf[i] && buf[i]<UBRK_SENTENCE_TERM_LIMIT)
                        indx.back().mark |= sentence_term;
                    else if(UBRK_SENTENCE_SEP<=buf[i] && buf[i]<UBRK_SENTENCE_SEP_LIMIT)
                        indx.back().mark |= sentence_sep;
                    break;
                default:
                    ;
                }
            }
        }
        else {
            indx.back().mark |=character_any; // Baisc mark... for character
        }
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
BOOSTER_API index_type 
map(boundary_type t,char const *begin,char const *end,std::locale const &loc)
{
    return do_map(t,begin,end,loc);
}

#ifndef BOOSTER_NO_STD_WSTRING
template<>
BOOSTER_API index_type 
map(boundary_type t,wchar_t const *begin,wchar_t const *end,std::locale const &loc)
{
    return do_map(t,begin,end,loc);
}
#endif

#ifdef BOOSTER_HAS_CHAR16_T
template<>
BOOSTER_API index_type 
map(boundary_type t,char16_t const *begin,char16_t const *end,std::locale const &loc)
{
    return do_map(t,begin,end,loc);
}
#endif

#ifdef BOOSTER_HAS_CHAR32_T
template<>
BOOSTER_API index_type 
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
