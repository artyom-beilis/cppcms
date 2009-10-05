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
	class convert_impl : public util::noncopyable {
	public:
		virtual std::string to_upper(std::string const &str) const = 0;
		virtual std::string to_lower(std::string const &str) const = 0;
		virtual std::string to_title(std::string const &str) const = 0;
		virtual std::string to_normal(std::string const &str,convert::norm_type how) const = 0;
		virtual ~convert_impl() {};
	};


	

#ifdef HAVE_ICU

	class icu_convert_impl : public convert_impl {
	public:
		icu_convert_impl(std::locale const &loc) :
			std_locale_(loc),
			icu_locale_(std::use_facet<icu_locale>(loc).get())
		{
		}

		static icu::UnicodeString normalize(icu::UnicodeString const &str,convert::norm_type how)
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

		std::string to_upper(std::string const &str) const
		{
			return impl::icu_to_std((impl::std_to_icu(str,std_locale_).toUpper(icu_locale_)),std_locale_);
		}
		std::string to_lower(std::string const &str) const
		{
			return impl::icu_to_std((impl::std_to_icu(str,std_locale_).toLower(icu_locale_)),std_locale_);
		}
		std::string to_title(std::string const &str) const
		{
			return impl::icu_to_std((impl::std_to_icu(str,std_locale_).toTitle(0,icu_locale_)),std_locale_);
		}
		std::string to_normal(std::string const &str,convert::norm_type how) const
		{
			return impl::icu_to_std(normalize(impl::std_to_icu(str,std_locale_),how),std_locale_);
		}
	private:
		friend class convert;
		std::locale std_locale_;
		icu::Locale icu_locale_;
	};


	

#endif

	class std_convert_impl : public convert_impl {
	public:
		
		
		std_convert_impl(std::locale const &l) : 
			locale_(l),
			cfacet_(0),
			wfacet_(0),
			charset_(0)
		{
			if(std::has_facet<std::ctype<wchar_t> >(locale_)) {
				wfacet_ = & std::use_facet<std::ctype<wchar_t> >(locale_);
				charset_ = & std::use_facet<charset>(locale_);
			}
			else {
				cfacet_ = & std::use_facet<std::ctype<char> >(locale_);
			}
		}

		template<typename Char>
		void titelize(std::basic_string<Char> &str,std::ctype<Char> const *conv) const
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
				std::wstring tmp=charset_->to_wstring(str);
				wfacet_->toupper(&tmp[0],&tmp[0]+tmp.size());
				return charset_->from_wstring(tmp);
			}
			else {
				std::string tmp=str;
				cfacet_->toupper(&tmp[0],&tmp[0]+tmp.size());
				return tmp;
			}
		}

		std::string to_lower(std::string const &str) const
		{
			if(wfacet_) {
				std::wstring tmp=charset_->to_wstring(str);
				wfacet_->tolower(&tmp[0],&tmp[0]+tmp.size());
				return charset_->from_wstring(tmp);
			}
			else {
				std::string tmp=str;
				cfacet_->tolower(&tmp[0],&tmp[0]+tmp.size());
				return tmp;
			}
		}
		
		std::string to_title(std::string const &str) const
		{
			if(wfacet_) {
				std::wstring tmp=charset_->to_wstring(str);
				titelize(tmp,wfacet_);
				return charset_->from_wstring(tmp);
			}
			else {
				std::string tmp=str;
				titelize(tmp,cfacet_);
				return tmp;
			}
		}

		std::string to_normal(std::string const &str,convert::norm_type how) const
		{
			#ifdef HAVE_ICU
			return impl::icu_to_std(icu_convert_impl::normalize(impl::std_to_icu(str,locale_),how),locale_);
			#else
			return str;
			#endif
		}
	private:
		std::locale locale_;
		std::ctype<char> const *cfacet_;
		std::ctype<wchar_t> const *wfacet_;
		charset const *charset_;
	}; // std_convert
	
	
	
	std::locale::id convert::id;

	convert *convert::create(std::locale const &l,std::string provider)
	{
		#ifdef HAVE_ICU
		char const *default_provider="icu";
		#else 
		char const *default_provider="std";
		#endif

		if(provider=="default")
			provider=default_provider;

		#ifdef HAVE_ICU
		if(provider=="icu") {
			return new convert(new icu_convert_impl(l));
		}
		#endif

		if(provider=="std") {
			return new convert(new std_convert_impl(l));
		}
		throw std::runtime_error("Unknown locale provider:"+provider);
			 
	}
	
	convert::convert(convert_impl *impl,size_t refs) : std::locale::facet(refs), impl_(impl)
	{
	}
	convert::~convert()
	{
	}

	std::string convert::to_upper(std::string const &str) const { return impl_->to_upper(str); }
	std::string convert::to_lower(std::string const &str) const { return impl_->to_lower(str); }
	std::string convert::to_title(std::string const &str) const { return impl_->to_title(str); }
	std::string convert::to_normal(std::string const &str,norm_type how) const { return impl_->to_normal(str,how); }


} /// locale
} /// convert

