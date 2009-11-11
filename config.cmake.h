#ifndef CPPCMS_CONFIG_H
#define CPPCMS_CONFIG_H
/* "Have C++0x std::uXXstring" */
#cmakedefine HAVE_CPP0X_UXSTRING

/* "Have C++0x auto" */
#cmakedefine HAVE_CPP_0X_AUTO

/* "Have C++0x decltype" */
#cmakedefine HAVE_CPP_0X_DECLTYPE

/* "Have g++ typeof" */
#cmakedefine HAVE_GCC_TYPEOF

/* "Enable ICU support" */
#cmakedefine HAVE_ICU

/* "Enable ICONV support" */
#cmakedefine HAVE_ICONV

/* "Have std::wstring" */
#cmakedefine HAVE_STD_WSTRING

/* "Have __sync_fetch_and_add" */
#cmakedefine HAVE_SYNC_FETCH_AND_ADD

/* "Have g++ typeof" */
#cmakedefine HAVE_UNDERSCORE_TYPEOF

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME "${PACKAGE_NAME}"

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING "${PACKAGE_STRING}"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "${PACKAGE_VERSION}"

/* The size of `wchar_t', as computed by sizeof. */
#cmakedefine SIZEOF_WCHAR_T ${SIZEOF_WCHAR_T}

/* Version number of package */
#define VERSION PACKAGE_VERSION

#endif
