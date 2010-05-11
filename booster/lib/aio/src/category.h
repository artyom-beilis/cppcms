#ifndef BOOSTER_AIO_SRC_CATEGORY_H
#define BOOSTER_AIO_SRC_CATEGORY_H

#include <booster/system_error.h>
#include <booster/config.h>

#ifdef BOOSTER_CYGWIN
static booster::system::error_category const &syscat = booster::system::get_windows_category();
#else
static booster::system::error_category const &syscat = booster::system::get_system_category();
#endif

#endif
