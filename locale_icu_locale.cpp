#define CPPCMS_SOURCE

#include "locale_icu_locale.h"
#ifdef HAVE_ICU
namespace cppcms { namespace locale {
	std::locale::id icu_locale::id;

	struct icu_locale::data {};

	icu_locale::icu_locale(std::string name,size_t refs) :
		 std::locale::facet(refs),
		 locale_(icu::Locale::createCanonical(name.c_str()))
	{
	}
	icu_locale::~icu_locale()
	{
	}
	icu::Locale const &icu_locale::get() const
	{
		return locale_;
	}
	
}} // locale::cppcms
#endif
