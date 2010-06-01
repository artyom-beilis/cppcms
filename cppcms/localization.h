#ifndef CPPCMS_LOCALIZATION_H
#define CPPCMS_LOCALIZATION_H

#include <cppcms/config.h>

#ifdef CPPCMS_HAVE_ICU
#	include <booster/locale.h>

	namespace cppcms {
		namespace locale = ::booster::locale;
	}
#else
#	include <cppcms/noicu_localization.h>
#endif


#endif
