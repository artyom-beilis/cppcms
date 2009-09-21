#ifndef CPPCMS_ICU_UTIL_H
#define CPPCMS_ICU_UTIL_H

#include "defs.h"
#include "config.h"
#include <string>
#include <locale>

#ifdef HAVE_ICU

#include <unicode/unistr.h>

namespace cppcms { namespace util {

	/// General basic prototype
	template<typename Char>
	std::basic_string<Char> icu_to_std(icu::UnicodeString const &str);

	template<typename Char>
	icu::UnicodeString std_to_icu(std::basic_string<Char> const &str);


	template<>
	std::basic_string<char> CPPCMS_API icu_to_std(icu::UnicodeString const &str);

	template<>
	icu::UnicodeString CPPCMS_API std_to_icu(std::basic_string<char> const &str);
	
	#ifdef HAVE_STD_WSTRING
	template<>
	std::basic_string<wchar_t> CPPCMS_API icu_to_std(icu::UnicodeString const &str);

	template<>
	icu::UnicodeString CPPCMS_API std_to_icu(std::basic_string<wchar_t> const &str);
	#endif

	#ifdef HAVE_CPP0X_UXSTRING
	
	template<>
	std::basic_string<char16_t> CPPCMS_API icu_to_std(icu::UnicodeString const &str);

	template<>
	icu::UnicodeString CPPCMS_API std_to_icu(std::basic_string<char16_t> const &str);
	
	template<>
	std::basic_string<char32_t> CPPCMS_API icu_to_std(icu::UnicodeString const &str);

	template<>
	icu::UnicodeString CPPCMS_API std_to_icu(std::basic_string<char32_t> const &str);
	
	#endif

	std::string CPPCMS_API icu_to_std(icu::UnicodeString const &str,std::locale const &l);
	icu::UnicodeString CPPCMS_API std_to_icu(std::string const &str,std::locale const &l);
	

#endif

} // util
} // cppcms


#endif
