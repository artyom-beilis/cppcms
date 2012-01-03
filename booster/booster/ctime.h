//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_CTIME_H
#define BOOSTER_CTIME_H

#include <booster/config.h>
#include <ctime>
#include <time.h>

namespace booster {
	///
	/// Convert POSIX time to local time. Effectivly works as C localtime
	///
	BOOSTER_API std::tm local_time(time_t pt);
	///
	/// Convert POSIX time to GMT time. Effectivly works as C gmtime
	///
	BOOSTER_API std::tm universal_time(time_t pt);

	///
	/// Converts local time std::tm \a t to POSIX time normalizing it, effectivly same as mktime
	///
	BOOSTER_API time_t normalize_local_time(std::tm &t);
	///
	/// Converts GMT time std::tm \a t to POSIX time normalizing it, effectivly same as timegm or mktime in case
	/// of GMT time zone.
	///
	BOOSTER_API time_t normalize_universal_time(std::tm &t);

	///
	/// Converts local time std::tm \a t to POSIX time, effectivly same as mktime but does not modify
	/// its parameter
	///
	BOOSTER_API time_t make_local_time(std::tm const &t);
	///
	/// Converts GMT time std::tm \a t to POSIX time , effectivly same as timegm or mktime in case
	/// of GMT time zone, but does not modify its parameter
	///
	BOOSTER_API time_t make_universal_time(std::tm const &t);
}

#endif

