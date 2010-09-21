//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/locale/formatting.h>
#include "formatter.h"
#include <booster/locale/info.h>
#include "uconv.h"


#include <unicode/numfmt.h>
#include <unicode/rbnf.h>
#include <unicode/datefmt.h>
#include <unicode/smpdtfmt.h>
#include <unicode/decimfmt.h>

#include <limits>

#include <iostream>

#include "predefined_formatters.h"

#ifdef BOOSTER_MSVC
#  pragma warning(disable : 4244) // loose data 
#endif


namespace booster {
namespace locale {
    namespace impl_icu {
        
        
        std::locale::id icu_formatters_cache::id;

        namespace {
            struct init { init() { std::has_facet<icu_formatters_cache>(std::locale::classic()); } } instance;
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
                icu_fmt_->format(::int32_t(value),tmp);
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
                int64_t v = 0;
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
                double v = 0;
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

        icu::UnicodeString strftime_to_icu_symbol(char c,icu::Locale const &locale,icu_formatters_cache const *cache=0)
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
                {
                    if(cache)
                        return cache->date_time_format_[1][1];
                    return strftime_to_icu_full(
                        icu::DateFormat::createDateTimeInstance(icu::DateFormat::kFull,icu::DateFormat::kFull,locale),
                        "YYYY-MM-dd HH:mm:ss"
                    );
                }
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
                return "hh:mm:ss a";
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
                {
                    if(cache)
                        return cache->date_format_[1];
                    return strftime_to_icu_full(
                        icu::DateFormat::createDateInstance(icu::DateFormat::kMedium,locale),
                        "YYYY-MM-dd"
                    );
                }
            case 'X': // Time
                {
                    if(cache)
                        return cache->time_format_[1];
                    return strftime_to_icu_full(
                        icu::DateFormat::createTimeInstance(icu::DateFormat::kMedium,locale),
                        "HH:mm:ss"
                    );
                }
            case 'y': // Year [00-99]
                return "YY";
            case 'Y': // Year 1998
                return "YYYY";
            case 'Z': // timezone
                return "vvvv";
            case '%': // %
                return "%";
            default:
                return "";
            }
        }

        icu::UnicodeString strftime_to_icu(icu::UnicodeString const &ftime,icu::Locale const &locale,
            icu_formatters_cache const *cache=0)
        {
            unsigned len=ftime.length();
            icu::UnicodeString result;
            bool escaped=false;
            for(unsigned i=0;i<len;i++) {
                UChar c=ftime[i];
                if(c=='%') {
                    i++;
                    c=ftime[i];
                    if(c=='E' || c=='O') {
                        i++;
                        c=ftime[i];
                    }
                    if(escaped) {
                        result+="'";
                        escaped=false;
                    }
                    result+=strftime_to_icu_symbol(c,locale);
                }
                else if(c=='\'') {
                        result+="''";
                }
                else {
                    if(!escaped) {
                        result+="'";
                        escaped=true;
                    }
                    result+=c;
                }
            }
            if(escaped)
                result+="'";
            return result;
        }
        
        template<typename CharType>
        std::auto_ptr<formatter<CharType> > generate_formatter(
                    std::ios_base &ios,
                    icu::Locale const &locale,
                    std::string const &encoding)
        {
            using namespace booster::locale::flags;

            std::auto_ptr<formatter<CharType> > fmt;
            ios_info &info=ios_info::get(ios);
            uint64_t disp = info.display_flags();

            icu_formatters_cache const &cache = std::use_facet<icu_formatters_cache>(ios.getloc());


            if(disp == posix)
                return fmt;
           
            UErrorCode err=U_ZERO_ERROR;
            
            switch(disp) {
            case number:
                {
                    std::ios_base::fmtflags how = (ios.flags() & std::ios_base::floatfield);
                    std::auto_ptr<icu::NumberFormat> nf;

                    if(how == std::ios_base::scientific)
                        nf.reset(static_cast<icu::NumberFormat*>(cache.number_format_scientific_->clone()));
                    else
                        nf.reset(static_cast<icu::NumberFormat*>(cache.number_format_->clone()));
                    
                    nf->setMaximumFractionDigits(ios.precision());
                    if(how == std::ios_base::scientific || how == std::ios_base::fixed ) {
                        nf->setMinimumFractionDigits(ios.precision());
                    }
                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case currency:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    
                    uint64_t curr = info.currency_flags();

                    if(curr == currency_default || curr == currency_national)
                        nf.reset(static_cast<icu::NumberFormat*>(cache.number_format_currency_national_->clone()));
                    else
                        nf.reset(static_cast<icu::NumberFormat*>(cache.number_format_currency_iso_->clone()));

                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case percent:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    nf.reset(static_cast<icu::NumberFormat*>(cache.number_format_percent_->clone()));
                    nf->setMaximumFractionDigits(ios.precision());
                    std::ios_base::fmtflags how = (ios.flags() & std::ios_base::floatfield);
                    if(how == std::ios_base::scientific || how == std::ios_base::fixed ) {
                        nf->setMinimumFractionDigits(ios.precision());
                    }
                    fmt.reset(new number_format<CharType>(nf,encoding));
                    
                }
                break;
            case spellout:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    nf.reset(static_cast<icu::NumberFormat*>(cache.number_format_spellout_->clone()));
                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case ordinal:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    nf.reset(static_cast<icu::NumberFormat*>(cache.number_format_ordinal_->clone()));
                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case date:
            case time:
            case datetime:
            case strftime:
                {
                    using namespace flags;
                    std::auto_ptr<icu::DateFormat> df;
                    // try to use cached first
                    if(cache.date_formatter_.get()) {
                        int tmf=info.time_flags();
                        switch(tmf) {
                        case time_short:
                            tmf=0;
                            break;
                        case time_long:
                            tmf=2;
                            break;
                        case time_full:
                            tmf=3;
                            break;
                        case time_default:
                        case time_medium:
                        default:
                            tmf=1;
                        }
                        int dtf=info.date_flags();
                        switch(dtf) {
                        case date_short:
                            dtf=0;
                            break;
                        case date_long:
                            dtf=2;
                            break;
                        case date_full:
                            dtf=3;
                            break;
                        case date_default:
                        case date_medium:
                        default:
                            dtf=1;
                        }

                        icu::UnicodeString pattern;
                        switch(disp) {
                        case date:
                            pattern = cache.date_format_[dtf];
                            break;
                        case time:
                            pattern = cache.time_format_[tmf];
                            break;
                        case datetime:
                            pattern = cache.date_time_format_[dtf][tmf];
                            break;
                        case strftime:
                            {
                                if( !cache.date_format_[1].isEmpty() 
                                    && !cache.time_format_[1].isEmpty()
                                    && !cache.date_time_format_[1][1].isEmpty())
                                {
                                    icu_std_converter<CharType> cvt_(encoding);
                                    std::basic_string<CharType> const &f=info.date_time_pattern<CharType>();
                                    pattern = strftime_to_icu(cvt_.icu(f.c_str(),f.c_str()+f.size()),locale,&cache);
                                }
                            }
                            break;
                        }
                        if(!pattern.isEmpty()) {
                            std::auto_ptr<icu::SimpleDateFormat> sdf(
                                static_cast<icu::SimpleDateFormat*>(cache.date_formatter_->clone()));
                            sdf->applyPattern(pattern);
                            df = sdf;
                        }
                    }
                    
                    if(!df.get()) {
                        icu::DateFormat::EStyle dstyle = icu::DateFormat::kDefault;
                        icu::DateFormat::EStyle tstyle = icu::DateFormat::kDefault;
                        
                        switch(info.time_flags()) {
                        case time_short:    tstyle=icu::DateFormat::kShort; break;
                        case time_medium:   tstyle=icu::DateFormat::kMedium; break;
                        case time_long:     tstyle=icu::DateFormat::kLong; break;
                        case time_full:     tstyle=icu::DateFormat::kFull; break;
                        }
                        switch(info.date_flags()) {
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
                            std::basic_string<CharType> const &f=info.date_time_pattern<CharType>();
                            icu::UnicodeString fmt = strftime_to_icu(cvt_.icu(f.data(),f.data()+f.size()),locale);
                            df.reset(new icu::SimpleDateFormat(fmt,locale,err));
                        }
                        if(!df.get())
                            return fmt;
                        if(U_FAILURE(err))
                            return fmt;
                    }

                    if(!info.time_zone().empty()) {
                        df->adoptTimeZone(icu::TimeZone::createTimeZone(info.time_zone().c_str()));
                    }
                        
                    fmt.reset(new date_format<CharType>(df,encoding));
                }
                break;
            }

            return fmt;
        }



    template<>
    std::auto_ptr<formatter<char> > formatter<char>::create(std::ios_base &ios,icu::Locale const &l,std::string const &e)
    {
        return generate_formatter<char>(ios,l,e);
    }

    template<>
    std::auto_ptr<formatter<wchar_t> > formatter<wchar_t>::create(std::ios_base &ios,icu::Locale const &l,std::string const &e)
    {
        return generate_formatter<wchar_t>(ios,l,e);
    }


    #ifdef BOOSTER_HAS_CHAR16_T
    template<>
    std::auto_ptr<formatter<char16_t> > formatter<char16_t>::create(std::ios_base &ios,icu::Locale const &l,std::string const &e)
    {
        return generate_formatter<char16_t>(ios,l,e);
    }

    #endif

    #ifdef BOOSTER_HAS_CHAR32_T
    template<>
    std::auto_ptr<formatter<char32_t> > formatter<char32_t>::create(std::ios_base &ios,icu::Locale const &l,std::string const &e)
    {
        return generate_formatter<char32_t>(ios,l,e);
    }

    #endif
    
} // impl_icu

} // locale
} // boost


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4



