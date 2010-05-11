#ifndef BOOSTER_CONFIG_H
#define BOOSTER_CONFIG_H

#if defined(__WIN32) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#	if defined(DLL_EXPORT)
#		if defined(BOOSTER_SOURCE) || defined(BOOSTER_LOCALE_SOURCE)
#			define BOOSTER_API __declspec(dllexport)
#		else
#			define BOOSTER_API __declspec(dllimport)
#		endif
#	else
#		define BOOSTER_API
#	endif
#else // ELF BINARIES
#	if defined(BOOSTER_SOURCE) && defined(BOOSTER_VISIBILITY_SUPPORT)
#		define BOOSTER_API __attribute__ ((visibility("default")))
#	else
#		define BOOSTER_API
#	endif
#endif

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32)) && !defined(__CYGWIN__)
#define BOOSTER_WIN_NATIVE
#endif

#if defined(__CYGWIN__)
#define BOOSTER_CYGWIN
#endif

#if defined(BOOSTER_WIN_NATIVE) || defined(BOOSTER_CYGWIN)
#define BOOSTER_WIN32
#endif

#if !defined(BOOSTER_WIN_NATIVE)
#define BOOSTER_POSIX
#endif

#if defined(_MSC_VER)
#define BOOSTER_MSVC
#endif


#undef BOOSTER_HAS_CHAR16_T
#undef BOOSTER_HAS_CHAR32_T
#undef BOOSTER_NO_STD_WSTRING
#undef BOOST_NO_SWPRINTF

#ifdef __GNUC__ 
#  define BOOSTER_GCC
#endif

#if defined(__GNUC__) && __GNUC__ < 4
#  define BOOSTER_GCC3
#endif

#if defined(__CYGWIN__) || (defined(BOOSTER_WIN32) && defined(BOOSTER_GCC3))
#  define BOOSTER_NO_STD_WSTRING
#endif

#if defined(BOOSTER_WIN32) && defined(BOOSTER_GCC)
#  define BOOST_NO_SWPRINTF 
#endif


#endif /// BOOSTER_CONFIG_H
