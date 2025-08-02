//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
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
	class windows_error_impl : public std::error_category {
	public:
		windows_error_impl() noexcept
		{
		}
		virtual const char* name() const noexcept override
		{
			return "windows";
		}
		virtual std::string message( int code ) const override
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

	error_category const &system_category() noexcept
	{
		static const windows_error_impl se;
		return se;
	}

} // system
} // booster

#endif // BOOSTER_WIN32


