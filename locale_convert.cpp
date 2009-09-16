#define CPPCMS_SOURCE
#include "locale_conver.h"

namespace cppcms {
namespace locale {
#ifdef HAVE_ICU
	struct convert::data {
		std::locale std_locale_;
		icu::Locale icu_locale_;
	};
	unicode_string convert::to_upper(unicode_string const &s) const
	{
		unicode_string tmp=s;
		tmp.toUpper(d->icu_locale_);
		return tmp;
	}
	unicode_string convert::to_lower(unicode_string const &s) const
	{
		unicode_string tmp=s;
		tmp.toLower(d->icu_locale_);
		return tmp;
	}
	unicode_string convert::to_title(unicode_string const &s) const
	{
		unicode_string tmp=s;
		tmp.toTitle(0,d->icu_locale_);
		return tmp;
	}
	unicode_string convert::to_normal(unicode_string const &s,norm_type how) const
	{
		UNormalizationMode mode;
		switch(how) {
		case norm_nfc: mode = UNORM_NFC; break;
		case norm_nfd: mode = UNORM_NFD; break;
		case norm_nfkc: mode = UNORM_NFKC; break;
		case norm_nfkd: mode = UNORM_NFKD; break;
		default: mode=UNORM_DEFAULT;			
		}

		unicode_string res;
		UErrorCode status = U_ZERO_ERROR;
		
		icu::Normalizer::normalize(s,mode,0,res,status);
		if(U_FAILURE(status)) 
			throw std::runtime_error(std::string("normalization failed:" + u_errorName(status)));
		return res;
	}

	template<>
	std::string convert::to_std(unicode_string const &s) const
	{
		return std::use_facet<charset>(d->std_locale_).from_icu_string(s);
	}

	template<>
	unicode_string convert::to_uni(std::string const &s) const
	{
		return std::use_facet<charset>(d->std_locale_).to_icu_string(s);
	}

	template<>
	std::basic_string<wchar_t> convert::to_std(unicode_string const &s) const
	{
		#if SIZEOF_WCHAR_T == 2
		return std::basic_string<wchar_t>(reinterpret_cast<wchar_t const *>(s.getBuffer()),s.length());
		#else
		std::basic_string<wchar_t> buf;
		buf.resize(s.length(),0);
		UErrorCode status = U_ZERO_ERROR;
		int size=s.toUTF32(&buf[0],buf.size(),status);
		buf.resize(size);
		return buf;
		#endif
	}

	template<>
	unicode_string convert::to_uni(std::basic_string<wchar_t> const &s) const
	{
		#if SIZEOF_WCHAR_T == 2
		return unicode_string(reinterpret_cast<UChar const *>(s.data()),s.size());
		#else
		return unicode_string::fromUTF32(reinterpret_cast<UChar32 const *>(s.data(),s.size()));
		#endif
	}

	template<>
	std::basic_string<uint16_t> convert::to_std(unicode_string const &s) const
	{
		return std::basic_string<uint16_t>(reinterpret_cast<uint16_t const *>(s.getbuffer()),s.length());
	}
	template<>
	unicode_string convert::to_uni(std::basic_string<uint16_t> const &s) const
	{
		return unicode_string(reinterpret_cast<UChar const *>(s.data()),s.size());
	}

	template<>
	std::basic_string<uint32_t> convert::to_std(unicode_string const &s) const
	{
		std::basic_string<uint32_t> buf;
		buf.resize(s.length(),0);
		UErrorCode status = U_ZERO_ERROR;
		int size=s.toUTF32(&buf[0],buf.size(),status);
		buf.resize(size);
		return buf;
	}

	template<>
	unicode_string convert::to_uni(std::basic_string<uint32_t> const &s) const
	{
		return unicode_string::fromUTF32(reinterpret_cast<UChar32 const *>(s.data(),s.size()));
	}

#ifdef HAVE_CPP0X_UXSTRING
	template<>
	std::basic_string<char16_t> convert::to_std(unicode_string const &s) const
	{
		return std::basic_string<char16_t>(reinterpret_cast<uint16_t const *>(s.getbuffer()),s.length());
	}
	template<>
	unicode_string convert::to_uni(std::basic_string<char16_t> const &s) const
	{
		return unicode_string(reinterpret_cast<UChar const *>(s.data()),s.size());
	}

	template<>
	std::basic_string<char32_t> convert::to_std(unicode_string const &s) const
	{
		std::basic_string<char32_t> buf;
		buf.resize(s.length(),0);
		UErrorCode status = U_ZERO_ERROR;
		int size=s.toUTF32(&buf[0],buf.size(),status);
		buf.resize(size);
		return buf;
	}

	template<>
	unicode_string convert::to_uni(std::basic_string<char32_t> const &s) const
	{
		return unicode_string::fromUTF32(reinterpret_cast<UChar32 const *>(s.data(),s.size()));
	}
#endif // C++0x

#else  // No ICU
#endif





} /// locale
} /// convert

