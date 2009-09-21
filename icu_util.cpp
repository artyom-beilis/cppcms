#define CPPCMS_SOURCE
#include "icu_util.h"
#ifdef HAVE_ICU

#include "locale_charset.h"
#include "utf_iterator.h"


namespace cppcms { namespace util {


namespace {
	template<typename Char>
	std::basic_string<Char> do_icu_to_std_utf8(icu::UnicodeString const &str)
	{
		std::basic_string<Char> tmp;
		tmp.reserve(str.length());
		uint16_t const *begin=reinterpret_cast<uint16_t const *>(str.getBuffer());
		uint16_t const *end=begin+str.length();
		while(begin<end) {
			uint32_t v=utf16::next(begin,end);
			if(v!=utf::illegal) {
				utf8::seq s=utf8::encode(v);
				tmp.append(s.c,s.len);
			}
		}
		return tmp;
	}

	template<typename Char>
	std::basic_string<Char> do_icu_to_std_utf16(icu::UnicodeString const &str)
	{
		return std::basic_string<Char>(reinterpret_cast<Char const *>(str.getBuffer()),str.length());
	}

	template<typename Char>
	std::basic_string<Char> do_icu_to_std_utf32(icu::UnicodeString const &str)
	{
		std::basic_string<Char> tmp;
		tmp.reserve(str.length());
		uint16_t const *begin=reinterpret_cast<uint16_t const *>(str.getBuffer());
		uint16_t const *end=begin+str.length();
		while(begin<end) {
			uint32_t v=utf16::next(begin,end);
			if(v!=utf::illegal)
				tmp+=(Char)(v);
		}
		return tmp;
	}

	template<typename Char>
	icu::UnicodeString do_std_utf8_to_icu(std::basic_string<Char> const &s)
	{
		icu::UnicodeString tmp(int32_t(s.size()),0,0);
		char const *begin=s.data();
		char const *end=begin+s.size();
		
		while(begin < end) {
			uint32_t v=utf8::next(begin,end);
			if(v!=utf::illegal)
				tmp.append((UChar32)(v));
		}
		return tmp;
	}

	template<typename Char>
	icu::UnicodeString do_std_utf16_to_icu(std::basic_string<Char> const &s)
	{
		return icu::UnicodeString(reinterpret_cast<UChar const *>(s.data()),s.size());
	}
	
	template<typename Char>
	icu::UnicodeString do_std_utf32_to_icu(std::basic_string<Char> const &s)
	{
		icu::UnicodeString tmp(s.size(),0,0);
		for(size_t i=0;i<s.size();i++)
			tmp.append((UChar32)(s[i]));
		return tmp;
	}

} // anon
	
	
	template<>
	std::basic_string<char> icu_to_std(icu::UnicodeString const &str) 
	{
		return do_icu_to_std_utf8<char>(str);
	}

	template<>
	icu::UnicodeString CPPCMS_API std_to_icu(std::basic_string<char> const &str)
	{
		return do_std_utf8_to_icu<char>(str);
	}
	
	#ifdef HAVE_STD_WSTRING
	template<>
	std::basic_string<wchar_t> CPPCMS_API icu_to_std(icu::UnicodeString const &str)
	{
		#if SIZEOF_WCHAR_T == 2
		return do_icu_to_std_utf16<wchar_t>(str);
		#else
		return do_icu_to_std_utf32<wchar_t>(str);
		#endif
	}

	template<>
	icu::UnicodeString CPPCMS_API std_to_icu(std::basic_string<wchar_t> const &str)
	{
		#if SIZEOF_WCHAR_T == 2
		return do_std_utf16_to_icu<wchar_t>(str);
		#else
		return do_std_utf32_to_icu<wchar_t>(str);
		#endif
	}

	#endif

	#ifdef HAVE_CPP0X_UXSTRING
	
	template<>
	std::basic_string<char16_t> CPPCMS_API icu_to_std(icu::UnicodeString const &str)
	{
		return do_icu_to_std_utf16<char16_t>(str);
	}

	template<>
	icu::UnicodeString CPPCMS_API std_to_icu(std::basic_string<char16_t> const &str)
	{
		return do_std_utf16_to_icu(str);
	}
	
	template<>
	std::basic_string<char32_t> CPPCMS_API icu_to_std(icu::UnicodeString const &str)
	{
		return do_icu_to_std_utf32<char32_t>(str);
	}

	template<>
	icu::UnicodeString CPPCMS_API std_to_icu(std::basic_string<char32_t> const &str)
	{
		return do_std_utf32_to_icu(str);
	}
	
	#endif

	std::string icu_to_std(icu::UnicodeString const &str,std::locale const &l)
	{
		return std::use_facet<locale::charset>(l).from_icu_string(str);
	}

	icu::UnicodeString std_to_icu(std::string const &str,std::locale const &l)
	{
		return std::use_facet<locale::charset>(l).to_icu_string(str);
	}

#endif

} } // cppcms::util
