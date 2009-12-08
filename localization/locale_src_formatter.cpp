//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_formatting.h"
#include "locale_formatter.h"
#include "locale_info.h"
#include "locale_src_ios_prop.hpp"
#include "locale_src_formatting_info.hpp"
#include "locale_src_uconv.hpp"
#include "locale_src_info_impl.hpp"


#include <unicode/numfmt.h>
#include <unicode/rbnf.h>
#include <unicode/datefmt.h>
#include <unicode/smpdtfmt.h>
#include <unicode/decimfmt.h>

#include <limits>

namespace cppcms {
namespace locale {
    namespace impl {
        
        typedef ios_prop<ios_info> ios_prop_info_type;

        namespace {
            struct  initializer {
                initializer()
                {
                    /// make sure xalloc called
                    ios_prop_info_type::global_init();
                }
            } init;
        }

        template<typename CharType>
        class number_format : public formatter<CharType> {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;
            
            virtual string_type format(double value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }
            virtual string_type format(int64_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }

            virtual string_type format(uint64_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(static_cast<int64_t>(value),tmp);
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }
            virtual string_type format(int32_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                #ifdef __SUNPRO_CC 
                icu_fmt_->format(static_cast<int>(value),tmp);
                #else
                icu_fmt_->format(value,tmp);
                #endif
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }
            virtual string_type format(uint32_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(static_cast<int64_t>(value),tmp);
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }

            virtual size_t parse(string_type const &str,double &value) const 
            {
                return do_parse(str,value);
            }

            virtual size_t parse(string_type const &str,int64_t &value) const 
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,uint32_t &value) const
            {
                int64_t v;
                size_t cut = do_parse(str,v);
                if(cut==0)
                    return 0;
                if(v < 0 || v > std::numeric_limits<uint32_t>::max())
                    return 0;
                value=static_cast<uint32_t>(v);
                return cut;
            }
            virtual size_t parse(string_type const &str,int32_t &value) const
            {
                return do_parse(str,value);
            }

            virtual size_t parse(string_type const &str,uint64_t &value) const
            {
                double v;
                size_t cut = do_parse(str,v);
                if(cut==0)
                    return 0;
                if(v < 0 || v > std::numeric_limits<uint64_t>::max() || static_cast<uint64_t>(v) != v)
                    return 0;
                value=static_cast<uint64_t>(v);
                return cut;
            }

            number_format(std::auto_ptr<icu::NumberFormat> fmt,std::string codepage) :
                cvt_(codepage),
                icu_fmt_(fmt)
            {
            }
 
        private:
            
            bool get_value(double &v,icu::Formattable &fmt) const
            {
                UErrorCode err=U_ZERO_ERROR;
                v=fmt.getDouble(err);
                if(U_FAILURE(err))
                    return false;
                return true;
            }

            bool get_value(int64_t &v,icu::Formattable &fmt) const
            {
                UErrorCode err=U_ZERO_ERROR;
                v=fmt.getInt64(err);
                if(U_FAILURE(err))
                    return false;
                return true;
            }

            bool get_value(int32_t &v,icu::Formattable &fmt) const
            {
                UErrorCode err=U_ZERO_ERROR;
                v=fmt.getLong(err);
                if(U_FAILURE(err))
                    return false;
                return true;
            }

            template<typename ValueType>
            size_t do_parse(string_type const &str,ValueType &v) const
            {
                icu::Formattable val;
                icu::ParsePosition pp;
                icu::UnicodeString tmp = cvt_.icu(str.data(),str.data()+str.size());

                icu_fmt_->parse(tmp,val,pp);

                ValueType tmp_v;

                if(pp.getIndex() == 0 || !get_value(tmp_v,val))
                    return 0;
                size_t cut = cvt_.cut(tmp,str.data(),str.data()+str.size(),pp.getIndex());
                if(cut==0)
                    return 0;
                v=tmp_v;
                return cut;
            }

            icu_std_converter<CharType> cvt_;
            std::auto_ptr<icu::NumberFormat> icu_fmt_;
        };
        
        
        template<typename CharType>
        class date_format : public formatter<CharType> {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;
            
            virtual string_type format(double value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }
            virtual string_type format(int64_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }

            virtual string_type format(uint64_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }
            virtual string_type format(int32_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }
            virtual string_type format(uint32_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }

            virtual size_t parse(string_type const &str,double &value) const 
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,int64_t &value) const 
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,uint64_t &value) const
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,int32_t &value) const
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,uint32_t &value) const
            {
                return do_parse(str,value);
            }

            date_format(std::auto_ptr<icu::DateFormat> fmt,std::string codepage) :
                cvt_(codepage),
                icu_fmt_(fmt)
            {
            }
 
        private:

            template<typename ValueType>
            size_t do_parse(string_type const &str,ValueType &value) const
            {
                icu::ParsePosition pp;
                icu::UnicodeString tmp = cvt_.icu(str.data(),str.data() + str.size());

                UDate udate = icu_fmt_->parse(tmp,pp);
                if(pp.getIndex() == 0)
                    return 0;
                double date = udate / 1000.0;
                typedef std::numeric_limits<ValueType> limits_type;
                if(date > limits_type::max() || date < limits_type::min())
                    return 0;
                size_t cut = cvt_.cut(tmp,str.data(),str.data()+str.size(),pp.getIndex());
                if(cut==0)
                    return 0;
                value=static_cast<ValueType>(date);
                return cut;

            }
            
            string_type do_format(double value,size_t &codepoints) const 
            {
                UDate date = value * 1000.0; /// UDate is time_t in miliseconds
                icu::UnicodeString tmp;
                icu_fmt_->format(date,tmp);
                codepoints=tmp.countChar32();
                return cvt_.std(tmp);
            }

            icu_std_converter<CharType> cvt_;
            std::auto_ptr<icu::DateFormat> icu_fmt_;
        };

        icu::UnicodeString strftime_to_icu_full(icu::DateFormat *dfin,char const *alt)
        {
            std::auto_ptr<icu::DateFormat> df(dfin);
            icu::SimpleDateFormat *sdf=dynamic_cast<icu::SimpleDateFormat *>(df.get());
            icu::UnicodeString tmp;
            if(sdf) {
                sdf->toPattern(tmp);
            }
            else {
                tmp=alt;
            }
            return tmp;

        }

        icu::UnicodeString strftime_to_icu_symbol(char c,icu::Locale const &locale)
        {
            switch(c) {
            case 'a': // Abbr Weekday
                return "EE";
            case 'A': // Full Weekday
                return "EEEE";
            case 'b': // Abbr Month
                return "MMM";
            case 'B': // Full Month
                return "MMMM";
            case 'c': // DateTile Full
                return strftime_to_icu_full(
                    icu::DateFormat::createDateTimeInstance(icu::DateFormat::kFull,icu::DateFormat::kFull,locale),
                    "YYYY-MM-dd HH:mm:ss"
                );
            // not supported by ICU ;(
            //  case 'C': // Century -> 1980 -> 19
            //  retur
            case 'd': // Day of Month [01,31]
                return "dd";
            case 'D': // %m/%d/%y
                return "MM/dd/YY";
            case 'e': // Day of Month [1,31]
                return "d";
            case 'h': // == b
                return "MMM";
            case 'H': // 24 clock hour 00,23
                return "HH";
            case 'I': // 12 clock hour 01,12
                return "hh";
            case 'j': // day of year 001,366
                return "D";
            case 'm': // month as [01,12]
                return "MM";
            case 'M': // minute [00,59]
                return "mm";
            case 'n': // \n
                return "\n";
            case 'p': // am-pm
                return "a";
            case 'r': // time with AM/PM %I:%M:%S %p
                return "HH:mm:ss a";
            case 'R': // %H:%M
                return "HH:mm";
            case 'S': // second [00,61]
                return "ss";
            case 't': // \t
                return "\t";
            case 'T': // %H:%M:%S
                return "HH:mm:ss";
/*          case 'u': // weekday 1,7 1=Monday
            case 'U': // week number of year [00,53] Sunday first
            case 'V': // week number of year [01,53] Moday first
            case 'w': // weekday 0,7 0=Sunday
            case 'W': // week number of year [00,53] Moday first, */
            case 'x': // Date
                return strftime_to_icu_full(
                    icu::DateFormat::createDateInstance(icu::DateFormat::kMedium,locale),
                    "YYYY-MM-dd"
                );
            case 'X': // Time
                return strftime_to_icu_full(
                    icu::DateFormat::createTimeInstance(icu::DateFormat::kMedium,locale),
                    "HH:mm:ss"
                );
            case 'y': // Year [00-99]
                return "YY";
            case 'Y': // Year 1998
                return "YYYY";
            case 'Z': // timezone
                return "VVVV";
            case '%': // %
                return "%";
            default:
                return "";
            }
        }

        icu::UnicodeString strftime_to_icu(icu::UnicodeString const &ftime,icu::Locale const &locale)
        {
            unsigned len=ftime.length();
            icu::UnicodeString result;
            for(unsigned i=0;i<len;i++) {
                UChar c=ftime[i];
                if(c=='%') {
                    i++;
                    c=ftime[i];
                    if(c=='E' || c=='O') {
                        i++;
                        c=ftime[i];
                    }
                    result+=strftime_to_icu_symbol(c,locale);
                }
                else if(('a'<=c && c<='z') || ('A'<=c && c<='Z')) {
                    result+="'";
                    while(('a'<=c && c<='z') || ('A'<=c && c<='Z')) {
                        result+=c;
                        i++;
                        c=ftime[i];
                    }
                    result+="'";
                }
                else if(c=='\'') {
                    result+="''";
                }
                else
                    result+=c;
            }
            return result;
        }
        
        template<typename CharType>
        std::auto_ptr<formatter<CharType> > generate_formatter(std::ios_base &ios)
        {
            using namespace cppcms::locale::flags;

            std::auto_ptr<formatter<CharType> > fmt;
            ios_info &info=ios_prop<ios_info>::get(ios);
            uint64_t disp = info.flags() &  flags::display_flags_mask;


            if(disp == posix)
                return fmt;
           
            cppcms::locale::info const &locale_info = std::use_facet<cppcms::locale::info>(ios.getloc());
            icu::Locale const &locale = locale_info.impl()->locale;
            std::string encoding = locale_info.impl()->encoding;


            UErrorCode err=U_ZERO_ERROR;
            
            switch(disp) {
            case number:
                {
                    std::ios_base::fmtflags how = (ios.flags() & std::ios_base::floatfield);
                    std::auto_ptr<icu::NumberFormat> nf;

                    if(how == std::ios_base::scientific)
                        nf.reset(icu::NumberFormat::createScientificInstance(locale,err));
                    else
                        nf.reset(icu::NumberFormat::createInstance(locale,err));
                    if(U_FAILURE(err)) {
                        return fmt;
                    }
                    if(how == std::ios_base::scientific || how == std::ios_base::fixed ) {
                        nf->setMaximumFractionDigits(ios.precision());
                        nf->setMinimumFractionDigits(ios.precision());
                    }
                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case currency:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    
                    #if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 402
                    //
                    // ICU 4.2 has special ISO currency style
                    //
                    
                    uint64_t curr = info.flags() & currency_flags_mask;

                    if(curr == currency_default || curr == currency_national)
                        nf.reset(icu::NumberFormat::createInstance(locale,icu::NumberFormat::kIsoCurrencyStyle,err));
                    else
                        nf.reset(icu::NumberFormat::createInstance(locale,icu::NumberFormat::kCurrencyStyle,err));

                    #else
                    //
                    // Before 4.2 we have no way to create ISO Currency 
                    //
                    
                    nf.reset(icu::NumberFormat::createCurrencyInstance(locale,err));

                    #endif

                    if(U_FAILURE(err))
                        return fmt;

                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case percent:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    nf.reset(icu::NumberFormat::createPercentInstance(locale,err));
                    if(U_FAILURE(err))
                        return fmt;
                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case spellout:
            case ordinal:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    URBNFRuleSetTag tag=URBNF_SPELLOUT;
                    if(disp==spellout)
                        tag=URBNF_SPELLOUT;
                    else // ordinal
                        tag=URBNF_ORDINAL;
                    nf.reset(new icu::RuleBasedNumberFormat(tag,locale,err));
                    if(U_FAILURE(err))
                        return fmt;
                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case date:
            case time:
            case datetime:
            case strftime:
                {
                    using namespace flags;
                    icu::DateFormat::EStyle dstyle = icu::DateFormat::kDefault,tstyle = icu::DateFormat::kDefault;
                    std::auto_ptr<icu::DateFormat> df;
                    
                    switch(info.flags() & time_flags_mask) {
                    case time_short:    tstyle=icu::DateFormat::kShort; break;
                    case time_medium:   tstyle=icu::DateFormat::kMedium; break;
                    case time_long:     tstyle=icu::DateFormat::kLong; break;
                    case time_full:     tstyle=icu::DateFormat::kFull; break;
                    }
                    switch(info.flags() & date_flags_mask) {
                    case date_short:    dstyle=icu::DateFormat::kShort; break;
                    case date_medium:   dstyle=icu::DateFormat::kMedium; break;
                    case date_long:     dstyle=icu::DateFormat::kLong; break;
                    case date_full:     dstyle=icu::DateFormat::kFull; break;
                    }
                    
                    if(disp==date)
                        df.reset(icu::DateFormat::createDateInstance(dstyle,locale));
                    else if(disp==time)
                        df.reset(icu::DateFormat::createTimeInstance(tstyle,locale));
                    else if(disp==datetime)
                        df.reset(icu::DateFormat::createDateTimeInstance(dstyle,tstyle,locale));
                    else {// strftime
                        icu_std_converter<CharType> cvt_(encoding);
                        std::basic_string<CharType> const &f=info.datetime<CharType>();
                        icu::UnicodeString fmt = strftime_to_icu(cvt_.icu(f.data(),f.data()+f.size()),locale);
                        df.reset(new icu::SimpleDateFormat(fmt,locale,err));
                    }
                    if(!df.get())
                        return fmt;
                    if(U_FAILURE(err))
                        return fmt;
                    if(!info.timezone().empty())
                        df->adoptTimeZone(icu::TimeZone::createTimeZone(info.timezone().c_str()));
                        
                    fmt.reset(new date_format<CharType>(df,encoding));
                }
                break;
            }

            return fmt;
        }

        template<typename Char>
        formatter<Char> const *get_cached_formatter(std::ios_base &ios)
        {
            return ios_prop<ios_info>::get(ios).formatter<Char>(ios);
        }


    } // impl

    template<>
    std::auto_ptr<formatter<char> > formatter<char>::create(std::ios_base &ios)
    {
        return impl::generate_formatter<char>(ios);
    }

    template<>
    formatter<char> const *formatter<char>::get(std::ios_base &ios)
    {
        return impl::get_cached_formatter<char>(ios);
    }
    #ifndef CPPCMS_NO_STD_WSTRING
    template<>
    std::auto_ptr<formatter<wchar_t> > formatter<wchar_t>::create(std::ios_base &ios)
    {
        return impl::generate_formatter<wchar_t>(ios);
    }

    template<>
    formatter<wchar_t> const *formatter<wchar_t>::get(std::ios_base &ios)
    {
        return impl::get_cached_formatter<wchar_t>(ios);
    }
    #endif

    #ifdef CPPCMS_HAS_CHAR16_T
    template<>
    std::auto_ptr<formatter<char16_t> > formatter<char16_t>::create(std::ios_base &ios)
    {
        return impl::generate_formatter<char16_t>(ios);
    }

    template<>
    formatter<char16_t> const *formatter<char16_t>::get(std::ios_base &ios)
    {
        return impl::get_cached_formatter<char16_t>(ios);
    }
    #endif

    #ifdef CPPCMS_HAS_CHAR32_T
    template<>
    std::auto_ptr<formatter<char32_t> > formatter<char32_t>::create(std::ios_base &ios)
    {
        return impl::generate_formatter<char32_t>(ios);
    }

    template<>
    formatter<char32_t> const *formatter<char32_t>::get(std::ios_base &ios)
    {
        return impl::get_cached_formatter<char32_t>(ios);
    }
    #endif

} // locale
} // boost


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4



