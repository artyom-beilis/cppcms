#ifndef CPPCMS_LOCALE_ICU_LOCALE_H
#define CPPCMS_LOCALE_ICU_LOCALE_H
#include "defs.h"
#include "config.h"
#include "hold_ptr.h"

#ifdef HAVE_ICU

#include <locale>
#include <string>
#include <unicode/locid.h>

namespace cppcms {
	namespace locale {

		class CPPCMS_API icu_locale : public std::locale::facet {
		public:
			static std::locale::id id;
			icu_locale(std::string name,size_t refs = 0);

			icu::Locale const &get() const;
			virtual ~icu_locale();
		private:
			struct data;
			util::hold_ptr<data> d;
			icu::Locale locale_;
		};
	}
}


#endif // HAVE_ICU
#endif
