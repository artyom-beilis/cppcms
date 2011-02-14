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
#cmakedefine CPPCMS_HAVE_STDINT_H

/* Have _atol64 */
#cmakedefine CPPCMS_HAVE_ATOI64

/* Have atoll */
#cmakedefine CPPCMS_HAVE_ATOLL

#if !defined(CPPCMS_HAVE_ATOLL) && defined(CPPCMS_HAVE_ATOI64)
#define atoll _atoi64
#endif

/* Have stat */

#cmakedefine CPPCMS_HAVE_STAT

/* Have _stat */

#cmakedefine CPPCMS_HAVE__STAT

/* Have tm.tm_zone */

#cmakedefine CPPCMS_HAVE_BSD_TM

/* Have snprintf */
#cmakedefine CPPCMS_HAVE_SNPRINTF


/* Have inttypes.h */
#cmakedefine CPPCMS_HAVE_INTTYPES_H

/* "Have C++0x std::uXXstring" */
#cmakedefine CPPCMS_HAVE_CPP0X_UXSTRING

#ifdef CPPCMS_HAVE_CPP0X_UXSTRING
# define CPPCMS_HAS_CHAR16_T
# define CPPCMS_HAS_CHAR32_T
#endif

/* "Have C++0x auto" */
#cmakedefine CPPCMS_HAVE_CPP_0X_AUTO

/* "Have C++0x decltype" */
#cmakedefine CPPCMS_HAVE_CPP_0X_DECLTYPE

/* "Have g++ typeof" */
#cmakedefine CPPCMS_HAVE_GCC_TYPEOF

/* "Enable ICU support" */
#cmakedefine CPPCMS_HAVE_ICU

/* Use STD locales instead of ICU ones */
#cmakedefine CPPCMS_DISABLE_ICU_LOCALIZATION

/* "Enable ICONV support" */
#cmakedefine CPPCMS_HAVE_ICONV

/* "Enable GNU GCrypt library */
#cmakedefine CPPCMS_HAVE_GCRYPT

/* "Enable OpenSSL library */
#cmakedefine CPPCMS_HAVE_OPENSSL

/* "Have std::wstring" */
#cmakedefine CPPCMS_HAVE_STD_WSTRING

#ifndef CPPCMS_HAVE_STD_WSTRING
# define CPPCMS_NO_STD_WSTRING
#endif 

/* Have canonicalize_file_name */

#cmakedefine CPPCMS_HAVE_CANONICALIZE_FILE_NAME

/* "Have g++ typeof" */
#cmakedefine CPPCMS_HAVE_UNDERSCORE_TYPEOF

/* Define to the full name of this package. */
#cmakedefine CPPCMS_PACKAGE_NAME "${CPPCMS_PACKAGE_NAME}"

/* Define to the full name and version of this package. */
#cmakedefine CPPCMS_PACKAGE_STRING "${CPPCMS_PACKAGE_STRING}"

/* Define to the version of this package. */
#cmakedefine CPPCMS_PACKAGE_VERSION "${CPPCMS_PACKAGE_VERSION}"

/* Define to module suffix. */
#cmakedefine CPPCMS_LIBRARY_SUFFIX "${CPPCMS_LIBRARY_SUFFIX}"

/* Define to module suffix. */
#cmakedefine CPPCMS_LIBRARY_PREFIX "${CPPCMS_LIBRARY_PREFIX}"

#cmakedefine CPPCMS_HAS_FCGI
#cmakedefine CPPCMS_HAS_SCGI
#cmakedefine CPPCMS_HAS_HTTP
#cmakedefine CPPCMS_NO_TCP_CACHE
#cmakedefine CPPCMS_NO_CACHE
#cmakedefine CPPCMS_NO_PREFOK_CACHE
#cmakedefine CPPCMS_NO_GZIP

#endif
