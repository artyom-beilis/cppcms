//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/config.h>
#include <booster/system_error.h>
#include <string.h>
#ifdef BOOSTER_WIN32
#include <sstream>
#include <iomanip>
#include <windows.h>

namespace booster { namespace system {
	class windows_error_impl : public error_category {
	public:
		virtual char const *name() const
		{
			return "windows";
		}
		virtual std::string message(int code) const
		{
			char *message=0;
			FormatMessageA(
				FORMAT_MESSAGE_FROM_SYSTEM
				|FORMAT_MESSAGE_ALLOCATE_BUFFER
				|FORMAT_MESSAGE_IGNORE_INSERTS,
				0,
				code,
				MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
				(char *)&message,
				0,
				0);
			std::string result;
			if(message != 0) {
				result = message;
				LocalFree(message);
			}
			else {
				std::ostringstream ss;
				ss<<"System error " << code << ", #" << std::hex << code;
				result = ss.str();
			}
			return result;
		}
	};

	error_category const &get_windows_category()
	{
		static const windows_error_impl se;
		return se;
	}
	
	#ifdef BOOSTER_WIN_NATIVE
	class system_error_impl : public windows_error_impl {
	public:
		virtual char const *name() const
		{
			return "system";
		}
	
	};
	error_category const &get_system_category()
	{
		static const system_error_impl se;
		return se;
	}
	#endif


} // system
} // booster

#endif // BOOSTER_WIN32

