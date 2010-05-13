#ifndef CPPCMS_LOCALE_FWD_H
#define CPPCMS_LOCALE_FWD_H

#include <cppcms/config.h>

#ifdef HAVE_ICU
	namespace booster {
		namespace locale {
			class generator;
			class info;
		}
	}
	namespace cppcms { 
		namespace locale = ::booster::locale; 
	}

#else
	namespace cppcms {
		namespace locale {
			class generator;
			class info;
		}
	}
#endif

#endif
