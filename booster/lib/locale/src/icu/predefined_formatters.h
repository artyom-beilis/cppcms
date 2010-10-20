//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_PREDEFINED_FORMATTERS_H_INCLUDED
#define BOOSTER_LOCALE_PREDEFINED_FORMATTERS_H_INCLUDED

#include <string>
#include <memory>
#include <booster/cstdint.h>
#include <booster/config.h>

#include <unicode/locid.h>
#include <unicode/numfmt.h>
#include <unicode/rbnf.h>
#include <unicode/datefmt.h>
#include <unicode/smpdtfmt.h>
#include <unicode/decimfmt.h>

namespace booster {
namespace locale {
    namespace impl_icu {        

        class icu_formatters_cache : public std::locale::facet {
        public:

            static std::locale::id id;

            icu_formatters_cache(icu::Locale const &locale)
            {
                UErrorCode err=U_ZERO_ERROR;
                number_format_.reset(icu::NumberFormat::createInstance(locale,err));
                test(err);
                number_format_scientific_.reset(icu::NumberFormat::createScientificInstance(locale,err));
                test(err);
                #if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 402
                number_format_currency_national_.reset(
                    icu::NumberFormat::createInstance(locale,icu::NumberFormat::kCurrencyStyle,err));
                test(err);
                number_format_currency_iso_.reset(
                    icu::NumberFormat::createInstance(locale,icu::NumberFormat::kIsoCurrencyStyle,err));
                test(err);
                #else
                number_format_currency_national_.reset(icu::NumberFormat::createCurrencyInstance(locale,err));
                test(err);
                number_format_currency_iso_.reset(icu::NumberFormat::createCurrencyInstance(locale,err));
                test(err);
                #endif
                number_format_percent_.reset(icu::NumberFormat::createPercentInstance(locale,err));
                test(err);
                number_format_spellout_.reset(new icu::RuleBasedNumberFormat(URBNF_SPELLOUT,locale,err));
                test(err);
                number_format_ordinal_.reset(new icu::RuleBasedNumberFormat(URBNF_ORDINAL,locale,err));
                test(err);

                static const icu::DateFormat::EStyle styles[4] = { 
                    icu::DateFormat::kShort,
                    icu::DateFormat::kMedium,
                    icu::DateFormat::kLong,
                    icu::DateFormat::kFull
                };

                std::auto_ptr<icu::DateFormat> fmt(icu::DateFormat::createDateTimeInstance(
                    icu::DateFormat::kMedium,
                    icu::DateFormat::kMedium,
                    locale));
                
                if(dynamic_cast<icu::SimpleDateFormat *>(fmt.get())) {
                    date_formatter_.reset(static_cast<icu::SimpleDateFormat *>(fmt.release()));
                }

                for(int i=0;i<4;i++) {
                    std::auto_ptr<icu::DateFormat> fmt(icu::DateFormat::createDateInstance(styles[i],locale));
                    icu::SimpleDateFormat *sfmt = dynamic_cast<icu::SimpleDateFormat*>(fmt.get());
                    if(sfmt) {
                        sfmt->toPattern(date_format_[i]);
                    }
                }

                for(int i=0;i<4;i++) {
                    std::auto_ptr<icu::DateFormat> fmt(icu::DateFormat::createTimeInstance(styles[i],locale));
                    icu::SimpleDateFormat *sfmt = dynamic_cast<icu::SimpleDateFormat*>(fmt.get());
                    if(sfmt) {
                        sfmt->toPattern(time_format_[i]);
                    }
                }

                for(int i=0;i<4;i++) {
                    for(int j=0;j<4;j++) {
                        std::auto_ptr<icu::DateFormat> fmt(
                            icu::DateFormat::createDateTimeInstance(styles[i],styles[j],locale));
                        icu::SimpleDateFormat *sfmt = dynamic_cast<icu::SimpleDateFormat*>(fmt.get());
                        if(sfmt) {
                            sfmt->toPattern(date_time_format_[i][j]);
                        }
                    }
                }


            }

            void test(UErrorCode err)
            {
                if(U_FAILURE(err))
                    throw booster::runtime_error("Failed to create a formatter");
            }

            std::auto_ptr<icu::NumberFormat>    number_format_;
            std::auto_ptr<icu::NumberFormat>    number_format_scientific_;
            std::auto_ptr<icu::NumberFormat>    number_format_currency_national_;
            std::auto_ptr<icu::NumberFormat>    number_format_currency_iso_;
            std::auto_ptr<icu::NumberFormat>    number_format_percent_;
            std::auto_ptr<icu::NumberFormat>    number_format_spellout_;
            std::auto_ptr<icu::NumberFormat>    number_format_ordinal_;

            std::auto_ptr<icu::SimpleDateFormat> date_formatter_;
            icu::UnicodeString date_format_[4];
            icu::UnicodeString time_format_[4];
            icu::UnicodeString date_time_format_[4][4];
        };



    } // namespace impl_icu
} // namespace locale
} // namespace boost



#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
