#ifndef CPPCMS_ICU_UTIL_H
#define CPPCMS_ICU_UTIL_H

#include "defs.h"
#include "config.h"
#include <string>
#include <locale>

#ifdef HAVE_ICU

#include <unicode/unistr.h>

namespace cppcms { 
	namespace impl {

		std::string icu_to_utf8(icu::UnicodeString const &str);
		icu::UnicodeString utf8_to_icu(std::string const &str);

		std::string icu_to_std(icu::UnicodeString const &str,std::locale const &l);
		icu::UnicodeString std_to_icu(std::string const &str,std::locale const &l);

	} // util
} // cppcms

#endif

#endif
