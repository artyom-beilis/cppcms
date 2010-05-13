#ifndef CPPCMS_LOCALIZATION_H
#define CPPCMS_LOCALIZATION_H

#include "config.h"

#ifdef HAVE_ICU
#	include <booster/locale.h>

	namespace cppcms {
		namespace locale = ::booster::locale;
	}
#else
#	include "noicu_localization.h"
#endif


#endif
