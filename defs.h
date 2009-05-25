#ifndef CPPCMS_DEFS_H
#define CPPCMS_DEFS_H

#if defined(__WIN32) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#	if defined(DLL_EXPORT)
#		ifdef CPPCMS_SOURCE
#			define CPPCMS_API __declspec(dllexport)
#		else
#			define CPPCMS_API __declspec(dllimport)
#		endif
#	else
#		define CPPCMS_API
#	endif
#else // ELF BINARIES
#	define CPPCMS_API __attribute__(visibility("default"))
#endif


#endif /// CPPCMS_DEFS_H
