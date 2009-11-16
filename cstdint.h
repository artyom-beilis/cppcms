#ifndef CPPCMS_CSTDINT_H
#define CPPCMS_CSTDINT_H
#include "config.h"
#if defined(HAVE_STDINT_H)
#include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#else 
// Generally only for broken MSVC
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#endif

#endif // CPPCMS_CSTDINT_H

