#define CPPCMS_SOURCE
#include "locale_numeric.h"
#include "icu_util.h"
#include "locale_icu_locale.h"
#include "cppcms_error.h"

#include <unicode/numfmt.h>
#include <unicode/rbnf.h>
#include <unicode/ustring.h>
#include <unicode/fmtable.h>

#include <boost/shared_ptr.hpp>

namespace cppcms {
namespace locale {
	
	class numeric_impl {
	public:
		typedef boost::shared_ptr<icu::NumberFormat> formatter_type;

		typedef numeric::format_type format_type;

		formatter_type get(format_type type,int presision=-1) const
		{
			formatter_type fmt;

			if(type < 0 || type >= numeric::format_count)
				type=numeric::format_normal;

			fmt=formatters_[type];

			if(presision == -1)
				return fmt;

			fmt.reset(static_cast<icu::NumberFormat *>(fmt->clone()));
			
			fmt->setMaximumFractionDigits(presision);
			fmt->setMinimumFractionDigits(presision);

			return fmt;
		}
		
		std::string format(format_type type,double v,int pres=-1) const
		{
			icu::UnicodeString str;
			return impl::icu_to_std(get(type,pres)->format(v,str),locale_);
		}
		
		std::string format(format_type type,int32_t v) const
		{
			icu::UnicodeString str;
			return impl::icu_to_std(get(type)->format(v,str),locale_);
		}

		std::string format(format_type type,int64_t v) const
		{
			icu::UnicodeString str;
			return impl::icu_to_std(get(type)->format(v,str),locale_);
		}

		bool parse_base(format_type type,std::string const &str,icu::Formattable &fmt) const
		{
			icu::ParsePosition pos;
			icu::UnicodeString ustr=impl::std_to_icu(str,locale_);

			get(type)->parse(ustr,fmt,pos);

			if(pos.getIndex()==0 || pos.getIndex()!=ustr.length())
				return false;
			return true;
		}


		bool parse(format_type type,std::string const &str,double &v) const
		{
			icu::Formattable fmt;
			if(!parse_base(type,str,fmt))
				return false;

			UErrorCode code=U_ZERO_ERROR;
			double tmp=fmt.getDouble(code);
			if(U_FAILURE(code))
				return false;
			v=tmp;
			return true;
		}
		
		bool parse(format_type type,std::string const &str,int32_t &v) const
		{
			icu::Formattable fmt;
			if(!parse_base(type,str,fmt))
				return false;

			UErrorCode code=U_ZERO_ERROR;
			int32_t tmp=fmt.getLong(code);
			if(U_FAILURE(code))
				return false;
			v=tmp;
			return true;
		}
		
		bool parse(format_type type,std::string const &str,int64_t &v) const
		{
			icu::Formattable fmt;
			if(!parse_base(type,str,fmt))
				return false;

			UErrorCode code=U_ZERO_ERROR;
			int64_t tmp=fmt.getInt64(code);
			if(U_FAILURE(code))
				return false;
			v=tmp;
			return true;
		}

		numeric_impl::numeric_impl(std::locale const &std_loc) : locale_(std_loc)
		{
			#define CPPCMS_ICU_VERSION (U_ICU_VERSION_MAJOR_NUM * 100 + U_ICU_VERSION_MINOR_NUM)
			
			icu::Locale const &icu_loc = std::use_facet<icu_locale>(locale_).get();

			UErrorCode err=U_ZERO_ERROR;
			
			formatters_[numeric::format_normal].reset(icu::NumberFormat::createInstance(icu_loc,err));
			check(err);

			formatters_[numeric::format_scientific].reset(icu::NumberFormat::createScientificInstance(icu_loc,err));
			check(err);

			formatters_[numeric::format_percent].reset(icu::NumberFormat::createPercentInstance(icu_loc,err));
			check(err);

			formatters_[numeric::format_currency].reset(icu::NumberFormat::createCurrencyInstance(icu_loc,err));
			check(err);

			#if CPPCMS_ICU_VERSION >= 402

			formatter_type fmt(icu::NumberFormat::createInstance(icu_loc,icu::NumberFormat::kIsoCurrencyStyle,err));

			#else

			formatter_type fmt(static_cast<icu::NumberFormat *>(formatters_[numeric::format_currency]->clone()));
			std::string iso_str = std::use_facet<std::moneypunct<char,true> >(locale_).curr_symbol();
			icu::UnicodeString iso=impl::std_to_icu(iso_str,locale_);
			fmt->setCurrency(iso.getBuffer(),err);
			
			#endif

			formatters_[numeric::format_iso_currency] = fmt;
			check(err);

			formatters_[numeric::format_spellout].reset(new icu::RuleBasedNumberFormat(URBNF_SPELLOUT,icu_loc,err));
			check(err);

			formatters_[numeric::format_ordinal].reset(new icu::RuleBasedNumberFormat(URBNF_ORDINAL,icu_loc,err));
			check(err);
			
			#if CPPCMS_ICU_VERSION >= 402

			formatters_[numeric::format_numbering].reset(new icu::RuleBasedNumberFormat(URBNF_NUMERING,icu_loc,err));

			#else

			formatters_[numeric::format_numbering] = formatters_[numeric::format_normal];

			#endif

			check(err);

		}
	private:
		void check(UErrorCode err)
		{
			if(U_FAILURE(err))
				throw cppcms_error(std::string("Failed to create NumericFormat") + u_errorName(err));
		}
		formatter_type formatters_[numeric::format_count];
		std::locale locale_;

	};


//// Proxy

	numeric::~numeric()
	{
	}
	
	numeric::numeric(numeric_impl *ptr,size_t refs) : std::locale::facet(refs), impl_(ptr) 
	{
	}
	
	std::locale::id numeric::id;

	std::string numeric::format(format_type type,double value) const
	{
		return impl_->format(type,value);
	}

	std::string numeric::format(format_type type,double value,int presision) const
	{
		return impl_->format(type,value,presision);
	}

	std::string numeric::format(format_type type,long long value) const
	{
		return impl_->format(type,int64_t(value));
	}
	std::string numeric::format(format_type type,unsigned long long value) const
	{
		return impl_->format(type,int64_t(value));
	}

	std::string numeric::format(format_type type,long value) const
	{
		return impl_->format(type,int64_t(value));
	}
	std::string numeric::format(format_type type,unsigned long value) const
	{
		return impl_->format(type,int64_t(value));
	}

	std::string numeric::format(format_type type,int value) const
	{
		return impl_->format(type,int32_t(value));
	}
	std::string numeric::format(format_type type,unsigned value) const
	{
		return impl_->format(type,int64_t(value));
	}
			
	bool numeric::parse(format_type type,std::string const &str,double &value) const
	{
		return impl_->parse(type,str,value);
	}
	bool numeric::parse(format_type type,std::string const &str,int &value) const
	{
		int32_t tmp;
		if(!impl_->parse(type,str,tmp)) return false;
		value=tmp;
		return true;
	}
	bool numeric::parse(format_type type,std::string const &str,unsigned int &value) const
	{
		int64_t tmp;
		if(!impl_->parse(type,str,tmp)) return false;
		value=tmp;
		return true;
	}

	bool numeric::parse(format_type type,std::string const &str,long &value) const
	{
		int64_t tmp;
		if(!impl_->parse(type,str,tmp)) return false;
		value=tmp;
		return true;
	}
	bool numeric::parse(format_type type,std::string const &str,unsigned long &value) const
	{
		int64_t tmp;
		if(!impl_->parse(type,str,tmp)) return false;
		value=tmp;
		return true;
	}

	bool numeric::parse(format_type type,std::string const &str,long long &value) const			
	{
		int64_t tmp;
		if(!impl_->parse(type,str,tmp)) return false;
		value=tmp;
		return true;
	}
	bool numeric::parse(format_type type,std::string const &str,unsigned long long &value) const
	{
		int64_t tmp;
		if(!impl_->parse(type,str,tmp)) return false;
		value=tmp;
		return true;
	}
	
	numeric *numeric::create(std::locale const &l)
	{
		return new numeric(new numeric_impl(l));
	}

} // locale
} // cppcms
