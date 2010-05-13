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


 


#endif
