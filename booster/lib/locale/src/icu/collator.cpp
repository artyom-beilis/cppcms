//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/locale/collator.h>
#include <booster/locale/generator.h>
#include <vector>
#include <limits>

#include "cdata.h"
#include "all_generator.h"
#include "uconv.h"
#include "../shared/mo_hash.h"

#include <unicode/coll.h>
#if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 402
#  include <unicode/stringpiece.h>
#endif

namespace booster {
    namespace locale {
        namespace impl_icu {
            template<typename CharType>
            class collate_impl : public collator<CharType> 
            {
            public:
                typedef typename collator<CharType>::level_type level_type;
                level_type limit(level_type level) const
                {
                    if(level < 0)
                        level=collator_base::primary;
                    else if(level >= level_count)
                        level = static_cast<level_type>(level_count - 1);
                    return level;
                }

                #if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 402
                int do_utf8_compare(    std::auto_ptr<icu::Collator> collate,
                                        level_type level,
                                        char const *b1,char const *e1,
                                        char const *b2,char const *e2,
                                        UErrorCode &status) const
                {
                    icu::StringPiece left (b1,e1-b1);
                    icu::StringPiece right(b2,e2-b2);
                    return collate->compareUTF8(left,right,status);

                }
                #endif
        
                int do_ustring_compare( std::auto_ptr<icu::Collator> collate,
                                        level_type level,
                                        CharType const *b1,CharType const *e1,
                                        CharType const *b2,CharType const *e2,
                                        UErrorCode &status) const
                {
                    icu::UnicodeString left=cvt_.icu(b1,e1);
                    icu::UnicodeString right=cvt_.icu(b2,e2);
                    return collate->compare(left,right,status);
                }
                
                int do_real_compare(std::auto_ptr<icu::Collator> collate,
                                    level_type level,
                                    CharType const *b1,CharType const *e1,
                                    CharType const *b2,CharType const *e2,
                                    UErrorCode &status) const
                {
                    return do_ustring_compare(collate,level,b1,e1,b2,e2,status);
                }

                virtual int do_compare( level_type level,
                                        CharType const *b1,CharType const *e1,
                                        CharType const *b2,CharType const *e2) const
                {
                    UErrorCode status=U_ZERO_ERROR;
                    
                    std::auto_ptr<icu::Collator> collate(collates_[limit(level)]->safeClone());
            
                    int res = do_real_compare(collate,level,b1,e1,b2,e2,status);
                    
                    if(U_FAILURE(status))
                            throw std::runtime_error(std::string("Collation failed:") + u_errorName(status));
                    if(res < 0)
                        return -1;
                    else if(res > 0)
                        return 1;
                    return 0;
                }
               
                std::vector<uint8_t> do_basic_transform(level_type level,CharType const *b,CharType const *e) const 
                {
                    icu::UnicodeString str=cvt_.icu(b,e);
                    std::vector<uint8_t> tmp;
                    tmp.resize(str.length());
                    std::auto_ptr<icu::Collator> collate(collates_[limit(level)]->safeClone());
                    int len = collate->getSortKey(str,&tmp[0],tmp.size());
                    if(len > int(tmp.size())) {
                        tmp.resize(len);
                        collate->getSortKey(str,&tmp[0],tmp.size());
                    }
                    else 
                        tmp.resize(len);
                    return tmp;
                }
                std::basic_string<CharType> do_transform(level_type level,CharType const *b,CharType const *e) const
                {
                    std::vector<uint8_t> tmp = do_basic_transform(level,b,e);
                    return std::basic_string<CharType>(tmp.begin(),tmp.end());
                }
                
                long do_hash(level_type level,CharType const *b,CharType const *e) const
                {
                    std::vector<uint8_t> tmp = do_basic_transform(level,b,e);
                    tmp.push_back(0);
                    return gnu_gettext::pj_winberger_hash_function(reinterpret_cast<char *>(&tmp.front()));
                }

                collate_impl(cdata const &d) : cvt_(d.encoding),is_utf8_(d.utf8)
                {
                
                    static const icu::Collator::ECollationStrength levels[level_count] = 
                    { 
                        icu::Collator::PRIMARY,
                        icu::Collator::SECONDARY,
                        icu::Collator::TERTIARY,
                        icu::Collator::QUATERNARY,
                        icu::Collator::IDENTICAL
                    };
                    
                    for(int i=0;i<level_count;i++) {

                        UErrorCode status=U_ZERO_ERROR;

                        collates_[i].reset(icu::Collator::createInstance(d.locale,status));

                        if(U_FAILURE(status))
                            throw std::runtime_error(std::string("Creation of collate failed:") + u_errorName(status));

                        collates_[i]->setStrength(levels[i]);
                    }
                }

            private:
                static const int level_count = 5;
                icu_std_converter<CharType>  cvt_;
                std::auto_ptr<icu::Collator> collates_[level_count];
                bool is_utf8_;
            };


            #if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 402
            template<>
            int collate_impl<char>::do_real_compare(    
                                    std::auto_ptr<icu::Collator> collate,
                                    level_type level,
                                    char const *b1,char const *e1,
                                    char const *b2,char const *e2,
                                    UErrorCode &status) const
            {
                if(is_utf8_)
                    return do_utf8_compare(collate,level,b1,e1,b2,e2,status);
                else
                    return do_ustring_compare(collate,level,b1,e1,b2,e2,status);
            }
            #endif
        
            std::locale create_collate(std::locale const &in,cdata const &cd,character_facet_type type)
            {
                switch(type) {
                case char_facet:
                    return std::locale(in,new collate_impl<char>(cd));
                case wchar_t_facet:
                    return std::locale(in,new collate_impl<wchar_t>(cd));
                #ifdef BOOSTER_HAS_CHAR16_T
                case char16_t_facet:
                    return std::locale(in,new collate_impl<char16_t>(cd));
                #endif
                #ifdef BOOSTER_HAS_CHAR32_T
                case char32_t_facet:
                    return std::locale(in,new collate_impl<char32_t>(cd));
                #endif
                default:
                    return in;
                }
            }

        } /// impl_icu

    } // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
