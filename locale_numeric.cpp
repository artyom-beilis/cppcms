#define CPPCMS_SOURCE_H
#include "locale_numeric.h"

namespace cppcms {
	namespace locale {
		num_put::iter_type num_put::do_put (num_put::iter_type out, std::ios_base& str, char_type fill, bool val) const
		{
			return std().put(out,str,fill,val);
		}
		num_put::iter_type num_put::do_put (num_put::iter_type out, std::ios_base& str, char_type fill, long val) const
		{
			if(use_std())
				return std().put(out,str,fill,val);
			return put(get_type(str),str.width().str.presision(),fill,out,int64_t(val));
		}
		num_put::iter_type num_put::do_put (num_put::iter_type out, std::ios_base& str, char_type fill, unsigned long val) const
		{
			if(use_std())
				return std().put(out,str,fill,val);
			return put(get_type(str),str.width().str.presision(),fill,out,int64_t(val));
		}
		num_put::iter_type num_put::do_put (num_put::iter_type out, std::ios_base& str, char_type fill, long long val) const
		{
			if(use_std())
				return std().put(out,str,fill,val);
			return put(get_type(str),str.width().str.presision(),fill,out,int64_t(val));
		}
		num_put::iter_type num_put::do_put (num_put::iter_type out, std::ios_base& str, char_type fill, unsigned long long val) const
		{
			if(use_std())
				return std().put(out,str,fill,val);
			return put(get_type(str),str.width().str.presision(),fill,out,int64_t(val));
		}
		num_put::iter_type num_put::do_put (num_put::iter_type out, std::ios_base& str, char_type fill, double val) const
		{
			return put(get_type(str,true),str.width().str.presision(),fill,out,val);
		}
		num_put::iter_type num_put::do_put (num_put::iter_type out, std::ios_base& str, char_type fill, long double val) const
		{
			return put(get_type(str,true),str.width().str.presision(),fill,out,double(val));
		}
		num_put::iter_type num_put::do_put (num_put::iter_type out, std::ios_base& str, char_type fill, const void* val) const
		{
			return std().put(out,str,fill,val);
		}

		num_put::iter_type put(format_type format,int width,int presision, char fill, num_put::iter_type out,int64_t value)
		{
			std::auto_ptr<icu::NumeberFormat> fmt;
			UErrorCode stat=U_ZERO_ERROR;
			switch(format) {
			case format_default:
				fmt.reset(icu::NumeberFormat::createInstance(stat));
				break;
			case format_currency:
				fmt.reset(icu::NumeberFormat::createCurrencyInstance(stat));
				break;
			case format_percent:
				fmt.reset(icu::NumeberFormat::createPercentInstance(stat));
				break;
			case format_spellout:
				fmt.reset(new icu::RuleBasedFormat::RuleBasedFormat(
				
			}
		}

	}
}
