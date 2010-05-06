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
		static const posix_error_impl se;
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
		static const system_error_impl se;
		return se;
	}



} // system
} // booster

#endif // POSIX
