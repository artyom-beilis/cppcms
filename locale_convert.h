#ifndef CPPCMS_LOCALE_CONVERT_H
#define CPPCMS_LOCALE_CONVERT_H

#include "defs.h"
#include <locale>

namespace cppcms {
namespace locale {
	class CPPCMS_API convert : public std::locale::facet {
	public:
		std::string to_upper(std::string const &s) const
		{
			return do_to_upper(s);
		}
		std::string to_lower(std::string const &s) const
		{
			return do_to_lower(s);
		}
		std::string to_title(std::string const &s) const
		{
			return do_to_title(s);
		}
	protected:

		virtual std::string to_upper(std::string const &s) const;
		virtual std::string to_lower(std::string const &s) const;
		virtual std::string to_title(std::string const &s) const;
	};	


} // locale
} // cppcms


#endif
