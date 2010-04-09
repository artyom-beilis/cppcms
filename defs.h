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
#	define CPPCMS_API
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
