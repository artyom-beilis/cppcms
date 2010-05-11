#ifndef BOOSTER_CSTDINT_H
#define BOOSTER_CSTDINT_H
#include <booster/config.h>

#if !defined(BOOSTER_MSVC) && !defined(__FreeBSD__)
#  include <stdint.h>
#elif defined(__FreeBSD__)
#  include <inttypes.h>
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

