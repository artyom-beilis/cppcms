//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_CSTDINT_H
#define BOOSTER_CSTDINT_H

#include <booster/build_config.h>

#if defined(BOOSTER_HAVE_STDINT_H) || defined(BOOSTER_HAVE_INTTYPES_H)

#  if defined BOOSTER_HAVE_STDINT_H
#     include <stdint.h>
#  elif defined BOOSTER_HAVE_INTTYPES_H
#     include <inttypes.h>
#  endif
	namespace booster {
		using ::int8_t;
		using ::uint8_t;
		using ::uint16_t;
		using ::int16_t;
		using ::uint32_t;
		using ::int32_t;
		using ::uint64_t;
		using ::int64_t;
	}


#else 
	namespace booster { 
		//
		// Generally only for broken MSVC
		// And guess
		typedef unsigned char uint8_t;
		typedef signed char int8_t;
		typedef unsigned short uint16_t;
		typedef short int16_t;
		typedef unsigned int uint32_t;
		typedef int int32_t;
		typedef unsigned long long uint64_t;
		typedef long long int64_t;
	}
#endif

#endif // BOOSTER_CSTDINT_H

