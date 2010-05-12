///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_CONFIG_H
#define CPPCMS_CONFIG_H

/* Have stdint.h */
#cmakedefine HAVE_STDINT_H

/* Have _atol64 */
#cmakedefine HAVE_ATOI64

/* Have atoll */
#cmakedefine HAVE_ATOLL

#if !defined(HAVE_ATOLL) && defined(HAVE_ATOI64)
#define atoll _atoi64
#endif

/* Have stat */

#cmakedefine HAVE_STAT

/* Have _stat */

#cmakedefine HAVE__STAT

/* Have gmtime_r */
#cmakedefine HAVE_GMTIME_R

/* Have localtime_r */
#cmakedefine HAVE_LOCALTIME_R

/* Have strerror_r */
#cmakedefine HAVE_STRERROR_R

/* Have tm.tm_zone */

#cmakedefine HAVE_BSD_TM

/* Have snprintf */
#cmakedefine HAVE_SNPRINTF


/* Have inttypes.h */
#cmakedefine HAVE_INTTYPES_H

/* "Have C++0x std::uXXstring" */
#cmakedefine HAVE_CPP0X_UXSTRING

#ifdef HAVE_CPP0X_UXSTRING
# define CPPCMS_HAS_CHAR16_T
# define CPPCMS_HAS_CHAR32_T
#endif

/* "Have C++0x auto" */
#cmakedefine HAVE_CPP_0X_AUTO

/* "Have C++0x decltype" */
#cmakedefine HAVE_CPP_0X_DECLTYPE

/* "Have g++ typeof" */
#cmakedefine HAVE_GCC_TYPEOF

/* "Enable ICU support" */
#cmakedefine HAVE_ICU

/* Use STD locales instead of ICU ones */
#cmakedefine CPPCMS_DISABLE_ICU_LOCALIZATION

/* "Enable ICONV support" */
#cmakedefine HAVE_ICONV

/* "Enable GNU GCrypt library */
#cmakedefine HAVE_GCRYPT

/* "Have std::wstring" */
#cmakedefine HAVE_STD_WSTRING

#ifndef HAVE_STD_WSTRING
# define CPPCMS_NO_STD_WSTRING
#endif 

/* Have canonicalize_file_name */

#cmakedefine HAVE_CANONICALIZE_FILE_NAME

/* "Have Atomic operations:" */
#cmakedefine HAVE_SYNC_FETCH_AND_ADD
#cmakedefine HAVE_GCC_BITS_EXCHANGE_AND_ADD
#cmakedefine HAVE_GCC_EXT_EXCHANGE_AND_ADD
#cmakedefine HAVE_FREEBSD_ATOMIC
#cmakedefine HAVE_SOLARIS_ATOMIC
#cmakedefine HAVE_MAC_OS_X_ATOMIC
#cmakedefine HAVE_WIN32_INTERLOCKED

/* "Have g++ typeof" */
#cmakedefine HAVE_UNDERSCORE_TYPEOF

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME "${PACKAGE_NAME}"

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING "${PACKAGE_STRING}"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "${PACKAGE_VERSION}"

/* Define to module suffix. */
#cmakedefine CPPCMS_LIBRARY_SUFFIX "${CPPCMS_LIBRARY_SUFFIX}"

/* Define to module suffix. */
#cmakedefine CPPCMS_LIBRARY_PREFIX "${CPPCMS_LIBRARY_PREFIX}"

#cmakedefine CPPCMS_USE_EXTERNAL_BOOST

#cmakedefine CPPCMS_HAS_FCGI
#cmakedefine CPPCMS_HAS_SCGI
#cmakedefine CPPCMS_HAS_HTTP

/* Version number of package */
#define VERSION PACKAGE_VERSION


#endif
