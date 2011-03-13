#ifndef CPPCMS_NOWIDE_CSTDIO_H
#define CPPCMS_NOWIDE_CSTDIO_H

#include <cstdio>
#include <stdio.h>
#include <booster/config.h>
#include <booster/nowide/convert.h>

#include <booster/config.h>
#ifdef BOOSTER_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4996)
#endif


namespace booster {
	namespace nowide {
	#ifndef BOOSTER_WIN_NATIVE
		using std::fopen;
		using std::freopen;
		using std::remove;
		using std::rename;
	#endif

	#if defined(BOOSTER_WIN_NATIVE) || defined(BOOSTER_DOXYGEN_DOCS)

		///
		/// Same as C fopen, but accepts UTF-8 string as file name under Windows
		///
		inline FILE *fopen(char const *file_name,char const *mode)
		{
			try {
				return _wfopen(convert(file_name).c_str(),convert(mode).c_str());
			}
			catch(bad_utf const &) {
				return 0;
			}
		}
		///
		/// Same as C freopen, but accepts UTF-8 string as file name under Windows
		///
		inline FILE *freopen(char const *file_name,char const *mode,FILE *stream)
		{
			try {
				return _wfreopen(convert(file_name).c_str(),convert(mode).c_str(),stream);
			}
			catch(bad_utf const &) {
				return 0;
			}
		}
		///
		/// Same as C rename, but accepts UTF-8 strings as file names under Windows
		///
		inline int rename(char const *old_name,char const *new_name)
		{
			try {
				return _wrename(convert(old_name).c_str(),convert(new_name).c_str());
			}
			catch(bad_utf const &) {
				return -1;
			}
		}
		///
		/// Same as C remove, but accepts UTF-8 string as file name under Windows
		///
		inline int remove(char const *name)
		{
			try {
				return _wremove(convert(name).c_str());
			}
			catch(bad_utf const &) {
				return -1;
			}
		}
	#endif
	} // nowide
} // booster

#ifdef BOOSTER_MSVC
#pragma warning(pop)
#endif

#endif
