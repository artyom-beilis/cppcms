#define CPPCMS_SOURCE
#include "locale_convert.h"
#include "locale_icu_locale.h"
#include "locale_charset.h"
#include "noncopyable.h"
#include "utf_iterator.h"

#ifdef HAVE_ICU
#include <unicode/unistr.h>
#include <unicode/normlzr.h>
#endif

#include <stdexcept>

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
		
		template<typename Char>
		std::basic_string<Char> to_std(icu::UnicodeString const &str) const
		{
			if(sizeof(Char) == 2) {
				return std::basic_string<Char>(reinterpret_cast<Char const *>(str.getBuffer()),str.length());
			}
			else if(sizeof(Char) == 4) {
				std::basic_string<Char> tmp;
				tmp.reserve(str.length());
				uint16_t *begin=reinterpret_cast<uint16_t const *>(str.getBuffer());
				uint16_t *end=begin+str.length();
				while(begin<end) {
					uint32_t v=utf16::next(begin,end);
					if(v!=utf::illegal)
						tmp+=(Char)(v);
				}
				return tmp;
			}
			else {
				throw std::bad_cast();
			}
		}

		template<typename Char>
		icu::UnicodeString to_uni(std::basic_string<Char> const &s) const
		{
			if(sizeof(Char) == 2)  {
				return icu::UnicodeString(reinterpret_cast<UChar const *>(s.data()),s.size());
			}
			else if(sizeof(Char) == 4)  {
				icu::UnicodeString tmp(s.size(),0,0);
				for(size_t i=0;i<s.size();i++)
					tmp.append((UChar32)(s[i]));
				return tmp;
			}
			else
				throw std::bad_cast();
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

	template<>
	std::basic_string<char> convert_impl::to_std(icu::UnicodeString const &str) const
	{
		return std::use_facet<charset>(std_locale_).from_icu_string(str);
	}
	template<>
	icu::UnicodeString convert_impl::to_uni(std::basic_string<char> const &str) const
	{
		return std::use_facet<charset>(std_locale_).to_icu_string(str);
	}


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
		return impl_->to_std<char>((impl_->to_uni(str).toUpper(impl_->icu_locale_)));
	}
	std::string convert::to_lower(std::string const &str) const
	{
		return impl_->to_std<char>((impl_->to_uni(str).toLower(impl_->icu_locale_)));
	}
	std::string convert::to_title(std::string const &str) const
	{
		return impl_->to_std<char>((impl_->to_uni(str).toTitle(0,impl_->icu_locale_)));
	}
	std::string convert::to_normal(std::string const &str,norm_type how) const
	{
		return impl_->to_std<char>(impl_->normalize(impl_->to_uni(str),how));
	}

	#ifdef HAVE_STD_WSTRING
	std::wstring convert::to_upper(std::wstring const &str) const
	{
		return impl_->to_std<wchar_t>((impl_->to_uni(str).toUpper(impl_->icu_locale_)));
	}
	std::wstring convert::to_lower(std::wstring const &str) const
	{
		return impl_->to_std<wchar_t>((impl_->to_uni(str).toLower(impl_->icu_locale_)));
	}
	std::wstring convert::to_title(std::wstring const &str) const
	{
		return impl_->to_std<wchar_t>((impl_->to_uni(str).toTitle(0,impl_->icu_locale_)));
	}
	std::wstring convert::to_normal(std::wstring const &str,norm_type how) const
	{
		return impl_->to_std<wchar_t>(impl_->normalize(impl_->to_uni(str),how));
	}
	#endif

	#ifdef HAVE_CPP0X_UXSTRING
	std::u16string convert::to_upper(std::u16string const &str) const
	{
		return impl_->to_std<char16_t>((impl_->to_uni(str).toUpper(impl_->icu_locale_)));
	}
	std::u16string convert::to_lower(std::u16string const &str) const
	{
		return impl_->to_std<char16_t>((impl_->to_uni(str).toLower(impl_->icu_locale_)));
	}
	std::u16string convert::to_title(std::u16string const &str) const
	{
		return impl_->to_std<char16_t>((impl_->to_uni(str).toTitle(0,impl_->icu_locale_)));
	}
	std::u16string convert::to_normal(std::u16string const &str,norm_type how) const
	{
		return impl_->to_std<char16_t>(impl_->normalize(impl_->to_uni(str),how));
	}
	std::u32string convert::to_upper(std::u32string const &str) const
	{
		return impl_->to_std<char32_t>((impl_->to_uni(str).toUpper(impl_->icu_locale_)));
	}
	std::u32string convert::to_lower(std::u32string const &str) const
	{
		return impl_->to_std<char32_t>((impl_->to_uni(str).toLower(impl_->icu_locale_)));
	}
	std::u32string convert::to_title(std::u32string const &str) const
	{
		return impl_->to_std<char32_t>((impl_->to_uni(str).toTitle(0,impl_->icu_locale_)));
	}
	std::u32string convert::to_normal(std::u32string const &str,norm_type how) const
	{
		return impl_->to_std<char32_t>(impl_->normalize(impl_->to_uni(str),how));
	}
	#endif
#else 
/////  NO ICU

	class convert_impl : public util::noncopyable {
	public:
		convert_impl(std::locale const &l) : locale_(l)
		{
		}

		template<typename Char>
		std::basic_string<Char> to_upper(std::basic_string<Char> const &str) const
		{
			if(std::had_facet<std::ctype<Char> >(locale_))
				return facet_to(str,&std::ctype<Char>::toupper,locale_);
			#ifdef HAVE_STD_WSTRING
			else if(std::has_facet<std::ctype<wchar_t>(locale_)) 
				return from_wstr<Char>(facet_to(to_wstr(str),&std::ctype<wchar_t>::toupper));
			#endif
			else
				return facet_to(str,&std::ctype<char>::toupper,std::locale::classic());
		}

		template<typename Char>
		std::basic_string<Char> to_lower(std::basic_string<Char> const &str) const
		{
			if(std::had_facet<std::ctype<Char> >(locale_))
				return facet_to(str,&std::ctype<Char>::tolower,locale_);
			#ifdef HAVE_STD_WSTRING
			else if(std::has_facet<std::ctype<wchar_t>(locale_)) 
				return from_wstr<Char>(facet_to(to_wstr(str),&std::ctype<wchar_t>::tolower));
			#endif
			else
				return facet_to(str,&std::ctype<char>::toupper,std::locale::classic());
		}
	

		template<typename Char>
		std::basic_string<Char> to_title(std::basic_string<Char> const &str) const
		{
			if(std::have_facet<ctype<Char> >(locale_)) {
				std::basic_string<Char> res = str;
				titelize(std::use_facet<std::ctype<Char> >(locale_),res);
			}
			#ifdef HAVE_STD_WSTRING
			else if(std::have_facet<ctype<wchar_t> >(locale_)) {
				std::wstring w=to_wstr(str);
				titelize(std::use_facet<std::ctype<wchar_t> >(locale_),w);
				return from_wstr<Char>(w);
			}
			#endif
			else {
				std::basic_string<Char> res = str;
				titleize(std::use_facet<std::ctype<char> >(std::locale::classic()),res);
				return res;
			}
		}



		#ifdef HAVE_STD_WSTRING
		template<typename Char>
		std::wstring to_wstr(std::basic_string<Char> const &str) const
		{
			if(sizeof(Char)==sizeof(wchar_t))
				return std::wstring(str.begin(),str.end());
			else if(sizeof(Char)==2 && sizeof(wchar_t)==4) {
				uint16_t const *begin=reinterpret_cast<uint16_t const *>(str.data());
				uint16_t const *end=begin+str.size();
				std::wstring tmp;
				tmp.reserve(str.size());
				uint32_t c;
				while(begin < end && (c = utf16::next(begin,end))!=utf::illegal)
					tmp.append(wchar_t(c));
				return tmp;
			}
			else if(sizeof(Char)==4 && sizeof(wchar_t)==2) {
				std::wstring tmp;
				tmp.reserve(str.size());
				for(unsigned i=0;i<str.size();i++) {
					utf16::seq s=utf16::encode(str[i]);
					tmp.append((wchar_t *)s.c,s.len);
				}
			}
			else
				throw std::bad_cast();
		}
		
		template<typename Char>
		std::basic_string<Char> from_wstr(std::wstring const &str) const
		{
			if(sizeof(Char)==sizeof(wchar_t))
				return std::basic_string<Char>(str.begin(),str.end());
			else if(sizeof(wchar_t)==2 && sizeof(Char)==4) {
				uint16_t const *begin=reinterpret_cast<uint16_t const *>(str.data());
				uint16_t const *end=begin+str.size();
				std::basic_string<Char> tmp;
				tmp.reserve(str.size());
				uint32_t c;
				while(begin < end && (c = utf16::next(begin,end))!=utf::illegal)
					tmp.append(Char(c));
				return tmp;
			}
			else if(sizeof(wchar_t)==4 && sizeof(Char)==2) {
				std::wstring tmp;
				tmp.reserve(str.size());
				for(unsigned i=0;i<str.size();i++) {
					utf16::seq s=utf16::encode(str[i]);
					tmp.append((Char *)s.c,s.len);
				}
			}
			else
				throw std::bad_cast();
		}
		#endif


		template<typename Char,typename CTypeChar>
		void titelize(std::ctype<CTypeChar> const &type,std::basic_string<Char> &res)
		{
			for(unsigned i=0;i<res.size();i++) {
				if(sizeof(CTypeChar) != sizeof(Char) ){
					if(res[i] < 0 || (i>0 && res[i-1] < 0))
						continue;
					if(res[i] > 0x7F || (i>0 && res[i-1] > 0x7F))
						continue;
				}
				if(	type.is(std::ctype_base::alpha,res[i]) 
					&& i>0
					&& type.is(std::ctype_base::space | std::ctype_base::punct,res[i-1]))
				{
					res[i]=type.toupper(res[i]);
				}
			}
		}

		template<typename Char,typename CharF>
		std::basic_string<Char> facet_to(
					std::basic_string<Char> const &str,
					CharF (std::ctype<CharF>::*to)(CharF) const,
					std::locale const &loc) const
		{
			std::basic_string<Char> res=str;
			std::ctype<CharF> const &conv=std::use_facet<std::ctype<CharF> >(loc);
			for(unsigned i=0;i<res.size();i++) {
				if(sizeof(Char) != sizeof(CharF)) 
					if(res[i] < 0 || res[i] > 0x7F)
						continue;
				res[i]=(conv.*to)(res[i]);
			}
			return res;
		}

		friend class convert;
		std::locale locale_;

	}

	
	#ifdef HAVE_STD_WSTRING
	template<>
	std::string convert_impl::to_upper(std::string const &str) const
	{

		if(!impl_->utf8_ || !std::has_facet<std::ctype<wchar_t> >(impl_->locale_)) {
			return facet_to(str,&std::ctype<char>::toupper)
		}
		else {
			return from_wstr<char>(facet_to(to_wstr(str),&std::ctype<wchar_t>::toupper));
		}
	}

	template<>	
	std::string convert_impl::to_lower(std::string const &str) const
	{

		if(!impl_->utf8_ || !std::has_facet<std::ctype<wchar_t> >(impl_->locale_)) {
			return facet_to(str,&std::ctype<char>::tolower)
		}
		else {
			return from_wstr<char>(facet_to(to_wstr(str),&std::ctype<wchar_t>::toupper));
		}
	}

	template<>
	std::wstring convert_impl::to_wstr(std::string const &str)
	{
		return std::use_facet<charset>(locale_).to_wstring(str);
	}

	template<>
	std::string convert_impl::from_wstr(std::wstring const &str)
	{
		return std::use_facet<charset>(locale_).from_wstring(str);
	}

	#endif
	
	
	
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
		return str;
	}

	#ifdef HAVE_STD_WSTRING
	std::wstring convert::to_upper(std::wstring const &str) const
	{
		return impl_->to_upper(str);
	}
	std::wstring convert::to_lower(std::wstring const &str) const
	{
		return impl_->to_lower(str);
	}
	std::wstring convert::to_title(std::wstring const &str) const
	{
		return impl_->to_title(str);
	}
	std::wstring convert::to_normal(std::wstring const &str,norm_type how) const
	{
		return str;
	}
	#endif


	#ifdef HAVE_CPP0X_UXSTRING
	std::u16string convert::to_upper(std::u16string const &str) const
	{
		return impl_->to_upper(str);
	}
	std::u16string convert::to_lower(std::u16string const &str) const
	{
		return impl_->to_lower(str);
	}
	std::u16string convert::to_title(std::u16string const &str) const
	{
		return impl_->to_title(str);
	}
	std::u16string convert::to_normal(std::u16string const &str,norm_type how) const
	{
		return str;
	}
	std::u32string convert::to_upper(std::u32string const &str) const
	{
		return impl_->to_upper(str);
	}
	std::u32string convert::to_lower(std::u32string const &str) const
	{
		return impl_->to_lower(str);
	}
	std::u32string convert::to_title(std::u32string const &str) const
	{
		return impl_->to_title(str);
	}
	std::u32string convert::to_normal(std::u32string const &str,norm_type how) const
	{
		return str;
	}
	#endif

#endif


} /// locale
} /// convert

