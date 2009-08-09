#define CPPCMS_SOURCE
#include "locale_info.h"
#include <boost/regex.hpp>


namespace cppcms {
namespace locale {

	std::locale::id info::id;

	struct info::data {};

	info::info(std::string name,std::size_t refs) : 
		std::locale::facet(refs),
		name_(name),
		is_utf8_(false)
	{
		static boost::regex lreg("^([a-zA-Z]+)(_([a-zA-Z])+)?(\\.([a-zA-Z0-9_\\-]+))?(\\@(.*))?$");
		boost::cmatch m;
		if(!boost::regex_match(name_.c_str(),m,lreg)) {
			return;
		}

		language_=m[1];
		territory_=m[3];
		encoding_=m[5];
		variant_=m[7];
		if(encoding_ == "utf-8" || encoding_=="UTF-8" || encoding_=="utf8" || encoding_=="UTF8")
			is_utf8_=true;
	}
	info::~info()
	{
	}
	
	std::string info::name() const { return name_; }
	std::string info::language() const { return language_; }
	std::string info::territory() const { return territory_; }
	std::string info::encoding() const { return encoding_; }
	std::string info::variant() const { return variant_; }
	bool info::is_utf8() const { return is_utf8_; }

} // locale
} // cppcms
