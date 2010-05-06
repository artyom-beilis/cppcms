#ifndef BOOSTER_AIO_SRC_CATEGORY_H
#define BOOSTER_AIO_SRC_CATEGORY_H

#ifndef BOOSTER_SYSTEM_ERROR_H
#  	error "Header <booster/system_error.h> must be included!"
#endif

#ifdef BOOSTER_CYGWIN
#	define system_category windows_category
#endif

#endif
