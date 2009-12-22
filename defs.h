#ifndef CPPCMS_DEFS_H
#define CPPCMS_DEFS_H

#if defined(__WIN32) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#	if defined(DLL_EXPORT)
#		if defined(CPPCMS_SOURCE) || defined(CPPCMS_LOCALE_SOURCE)
#			define CPPCMS_API __declspec(dllexport)
#		else
#			define CPPCMS_API __declspec(dllimport)
#		endif
#	else
#		define CPPCMS_API
#	endif
#else // ELF BINARIES
#	define CPPCMS_API __attribute__((visibility("default")))
#endif

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32)) && !defined(__CYGWIN__)
#define CPPCMS_WIN_NATIVE
#endif
#if defined(__CYGWIN__)
#define CPPCMS_CYGWIN
#endif

#if defined(CPPCMS_WIN_NATIVE) || defined(CPPCMS_CYGWIN)
#define CPPCMS_WIN32
#endif

#if !defined(CPPCMS_WIN_NATIVE)
#define CPPCMS_POSIX
#endif

#endif /// CPPCMS_DEFS_H
