#define CPPCMS_SOURCE
#include "locale_convert.h"
#include "locale_icu_locale.h"
#include "locale_charset.h"
#include "noncopyable.h"
#include "utf_iterator.h"

#ifdef HAVE_ICU
#include <unicode/unistr.h>
#include <unicode/normlzr.h>
#include "icu_util.h"
#endif

#include <stdexcept>
#include <typeinfo>

namespace cppcms {
namespace locale {
#ifdef HAVE_ICU

	class convert_impl : public util::noncopyable {
	public:
		convert_impl(std::locale const &loc) :
			std_locale_(loc),
			icu_locale_(std::use_facet<icu_locale>(loc).get())
		{
		}

		icu::UnicodeString normalize(icu::UnicodeString const &str,convert::norm_type how) const
		{
			UNormalizationMode mode;
			switch(how) {
			case convert::norm_nfc: mode = UNORM_NFC; break;
			case convert::norm_nfd: mode = UNORM_NFD; break;
			case convert::norm_nfkc: mode = UNORM_NFKC; break;
			case convert::norm_nfkd: mode = UNORM_NFKD; break;
			default: mode=UNORM_DEFAULT;
			}

			icu::UnicodeString res;
			UErrorCode status = U_ZERO_ERROR;

			icu::Normalizer::normalize(str,mode,0,res,status);
			if(U_FAILURE(status))
				throw std::runtime_error(std::string("normalization failed:") + u_errorName(status));
			return res;
		}
	private:
		friend class convert;
		std::locale std_locale_;
		icu::Locale icu_locale_;
	};


	std::locale::id convert::id;

	convert::convert(std::locale const &l,size_t refs) : 
		std::locale::facet(refs),
		impl_(new convert_impl(l))
	{
	}
	convert::~convert()
	{
	}
	
	std::string convert::to_upper(std::string const &str) const
	{
		return impl::icu_to_std((impl::std_to_icu(str).toUpper(impl_->icu_locale_)));
	}
	std::string convert::to_lower(std::string const &str) const
	{
		return impl::icu_to_std((impl::std_to_icu(str).toLower(impl_->icu_locale_)));
	}
	std::string convert::to_title(std::string const &str) const
	{
		return impl::icu_to_std((impl::std_to_icu(str).toTitle(0,impl_->icu_locale_)));
	}
	std::string convert::to_normal(std::string const &str,norm_type how) const
	{
		return impl::icu_to_std(impl_->normalize(impl::std_to_icu(str),how));
	}

#else /////  NO ICU

	class convert_impl : public util::noncopyable {
	public:
		
		
		convert_impl(std::locale const &l) : 
			locale_(l),
			cfacet_(0),
			wfacet_(0),
			charset_(0)
		{
			if(std::have_facet<std::ctype<wchar_t> >(locale_)) {
				wfacet_ = & std::use_facet<std::ctype<wchar_t> >(locale_);
				charset_ = & std::use_facet<charset>(locale_);
			}
			else {
				cfacet_ = & std::use_facet<std::ctype<char> >(locale_);
			}
		}

		template<typename Char>
		void titelize(std::basic_string<Char> &str,std::ctype<Char> const *conv)
		{
			bool prev_is_not_alpha = true;
			for(unsigned i=0;i<str.size();i++) {
				Char c=str[i];
				if(prev_is_not_alpha)
					str[i]=conv->toupper(c);
				else
					str[i]=conv->tolower(c);
				prev_is_not_alpha=!conv->is(std::ctype_base::alpha,c);
			}
		}
		
		std::string to_upper(std::string const &str) const
		{
			if(wfacet_) {
				std::wstring tmp=charset_->to_wstring(str)
				wfacet_->toupper(&tmp[0],&tmp[0]+tmp.size());
				return charset_->from_wstring(tmp);
			}
			std::string tmp=str;
			facet_->toupper(&tmp[0],&tmp[0]+tmp.size());
			return tmp;
		}

		std::string to_lower(std::string const &str) const
		{
			if(wfacet_) {
				std::wstring tmp=charset_->to_wstring(str)
				wfacet_->tolower(&tmp[0],&tmp[0]+tmp.size());
				return charset_->from_wstring(tmp);
			}
			std::string tmp=str;
			facet_->tolower(&tmp[0],&tmp[0]+tmp.size());
			return tmp;
		}
		
		std::string to_title(std::string const &str) const
		{
			if(wfacet_) {
				std::wstring tmp=charset_->to_wstring(str)
				titelize(tmp,wfacet_);
				return charset_->from_wstring(tmp);
			}
			else {
				std::string tmp=str;
				titelize(tmp,cfacet_);
				return tmp;
			}
		}
		std::string to_normal(std::string const &str,norm_type how) const
		{
			return str;
		}
	
#endif  /// ICU / NO ICU

	std::string convert::to_upper(std::string const &str) const
	{
		return impl_->to_upper(str);
	}
	std::string convert::to_lower(std::string const &str) const
	{
		return impl_->to_lower(str);
	}
	std::string convert::to_title(std::string const &str) const
	{
		return impl_->to_title(str);
	}
	std::string convert::to_normal(std::string const &str,norm_type how) const
	{
		return impl_->to_normal(str,how);
	}


} /// locale
} /// convert

