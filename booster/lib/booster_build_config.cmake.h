//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_BUILD_CONFIG_H
#define BOOSTER_BUILD_CONFIG_H

//
// GCC's __sync_* operations
//
#cmakedefine BOOSTER_HAS_GCC_SYNC

//
// STDC++ library atomic ops in <bits/atomicity.h>
//

#cmakedefine BOOSTER_HAVE_GCC_BITS_EXCHANGE_AND_ADD

//
// STDC++ library atomic ops in <ext/atomicity.h>
//

#cmakedefine BOOSTER_HAVE_GCC_EXT_EXCHANGE_AND_ADD

//
// FreeBSD atomic operations
//

#cmakedefine BOOSTER_HAVE_FREEBSD_ATOMIC

//
// Solaris atomic operations
//

#cmakedefine BOOSTER_HAVE_SOLARIS_ATOMIC

//
// Mac OS X atomic operations
//

#cmakedefine BOOSTER_HAVE_MAC_OS_X_ATOMIC

//
// Have <stdint.h>
//

#cmakedefine BOOSTER_HAVE_STDINT_H

//
// Have <inttypes.h>
//

#cmakedefine BOOSTER_HAVE_INTTYPES_H

//
// Have IPv6 support
//

#cmakedefine BOOSTER_AIO_HAVE_PF_INET6
 
#cmakedefine BOOSTER_HAVE_EXECINFO

/* Define to module suffix. */
#cmakedefine BOOSTER_LIBRARY_SUFFIX "${BOOSTER_LIBRARY_SUFFIX}"

/* Define to module suffix. */
#cmakedefine BOOSTER_LIBRARY_PREFIX "${BOOSTER_LIBRARY_PREFIX}"

#endif
