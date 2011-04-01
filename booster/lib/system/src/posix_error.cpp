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
#ifdef BOOSTER_POSIX
#include <string.h>

namespace booster { namespace system {
	class posix_error_impl : public error_category {
	public:
		virtual char const *name() const
		{
			return "posix";
		}
		virtual std::string message(int code) const
		{
			char buf[256]={0};
			return wrap(::strerror_r(code,buf,sizeof(buf)),buf);
		}
		static char const *wrap(int n,char *buf)
		{
			if(n!=0)
				return "Unknown";
			else
				return buf;
		}
		static char const *wrap(char *s,char * /*buf*/)
		{
			if(s==0)
				return "Unknown";
			else
				return s;
		}
	};

	error_category const &get_posix_category()
	{
		static const posix_error_impl se = posix_error_impl();
		return se;
	}

	class system_error_impl : public posix_error_impl {
	public:
		virtual char const *name() const
		{
			return "system";
		}
	
	};
	error_category const &get_system_category()
	{
		static const system_error_impl se = system_error_impl();
		return se;
	}



} // system
} // booster

#endif // POSIX
