#define CPPCMS_SOURCE
#include "locale_conver.h"
#include "config.h"

#ifdef CPPCMS_USE_ICU



#else
namespace cppcms {
namespace locale {
	std::string convert::to_upper(std::string const &s) const
	{
		
		ctype_facet_->
	}

} // locale
} // cppcms



#endif


