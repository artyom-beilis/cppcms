//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_collator.h"
#include "locale_info.h"
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/shared_ptr.hpp>
#   include <boost/functional/hash.hpp>
#else // Internal Boost
#   include <cppcms_boost/shared_ptr.hpp>
#   include <cppcms_boost/functional/hash.hpp>
    namespace boost = cppcms_boost;
#endif
#include <vector>
#include <limits>

#include "locale_src_info_impl.hpp"
#include "locale_src_uconv.hpp"

#include <unicode/coll.h>

namespace cppcms {
    namespace locale {
        namespace impl {
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
                
                virtual int do_compare( level_type level,
                                        CharType const *b1,CharType const *e1,
                                        CharType const *b2,CharType const *e2) const
                {
                    icu::UnicodeString left=cvt_.icu(b1,e1);
                    icu::UnicodeString right=cvt_.icu(b2,e2);
                    UErrorCode status=U_ZERO_ERROR;
                    int res = collates_[limit(level)]->compare(left,right,status);
                    if(U_FAILURE(status))
                            throw std::runtime_error(std::string("Collation failed:") + u_errorName(status));
                    if(res < 0)
                        return -1;
                    else if(res > 0)
                        return 1;
                    return 0;
                }
                
                std::basic_string<CharType> do_transform(level_type level,CharType const *b,CharType const *e) const
                {
                    icu::UnicodeString str=cvt_.icu(b,e);
                    std::vector<char> tmp;
                    tmp.resize(str.length());
                    boost::shared_ptr<icu::Collator> collate=collates_[limit(level)];
                    int len = collate->getSortKey(str,reinterpret_cast<uint8_t *>(&tmp[0]),tmp.size());
                    if(len > int(tmp.size())) {
                        tmp.resize(len);
                        collate->getSortKey(str,reinterpret_cast<uint8_t *>(&tmp[0]),tmp.size());
                    }
                    else 
                        tmp.resize(len);

                    if( std::numeric_limits<CharType>::min() == std::numeric_limits<char>::min() // Both unsigned or same
                        || (std::numeric_limits<CharType>::min() < 0 && std::numeric_limits<char>::min() < 0)) // both signed
                    {
                        return std::basic_string<CharType>(tmp.begin(),tmp.end());
                    }
                    else {
                        std::basic_string<CharType> out(tmp.size(),0);
                        for(unsigned i=0;i<tmp.size();i++)
                            out[i]=tmp[i]+128;
                        return out;
                    }
                }
                
                long do_hash(level_type level,CharType const *b,CharType const *e) const
                {
                    boost::hash<std::basic_string<CharType> > hasher;
                    return hasher(do_transform(level,b,e));
                }

                collate_impl(icu::Locale const &locale,std::string encoding) : cvt_(encoding)
                {
                
                    static const icu::Collator::ECollationStrength levels[level_count] = 
                    { 
                        icu::Collator::PRIMARY,
                        icu::Collator::SECONDARY,
                        icu::Collator::TERTIARY,
                        icu::Collator::QUATERNARY
                    };
                    
                    for(int i=0;i<level_count;i++) {

                        UErrorCode status=U_ZERO_ERROR;

                        collates_[i].reset(icu::Collator::createInstance(locale,status));

                        if(U_FAILURE(status))
                            throw std::runtime_error(std::string("Creation of collate failed:") + u_errorName(status));

                        collates_[i]->setStrength(levels[i]);
                    }
                }

            private:
                static const int level_count = 4;
                icu_std_converter<CharType>  cvt_;
                boost::shared_ptr<icu::Collator> collates_[level_count];
            };

        } /// impl

        template<>
        CPPCMS_API collator<char> *collator<char>::create(info const &inf)
        {
            return new impl::collate_impl<char>(inf.impl()->locale,inf.impl()->encoding);
        }
        #ifndef CPPCMS_NO_STD_WSTRING
        template<>
        CPPCMS_API collator<wchar_t> *collator<wchar_t>::create(info const &inf)
        {
            return new impl::collate_impl<wchar_t>(inf.impl()->locale,inf.impl()->encoding);
        }
        #endif
        
        #ifdef CPPCMS_HAS_CHAR16_T
        template<>
        CPPCMS_API collator<char16_t> *collator<char16_t>::create(info const &inf)
        {
            return new impl::collate_impl<char16_t>(inf.impl()->locale,inf.impl()->encoding);
        }
        #endif
        
        #ifdef CPPCMS_HAS_CHAR32_T
        template<>
        CPPCMS_API collator<char32_t> *collator<char32_t>::create(info const &inf)
        {
            return new impl::collate_impl<char32_t>(inf.impl()->locale,inf.impl()->encoding);
        }
        #endif




    } // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
