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

#cmakedefine BOOSTER_HAVE_UNWIND_BACKTRACE

#cmakedefine BOOSTER_HAVE_UNWIND_BACKTRACE_BUILTIN
/* Define to module suffix. */
#cmakedefine BOOSTER_LIBRARY_SUFFIX "${BOOSTER_LIBRARY_SUFFIX}"

/* Define to module suffix. */
#cmakedefine BOOSTER_LIBRARY_PREFIX "${BOOSTER_LIBRARY_PREFIX}"

#endif
