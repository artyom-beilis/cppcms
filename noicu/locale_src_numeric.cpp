#include "locale_numeric.h"
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/regex.hpp>
#else // Internal Boost
#   include <cppcms_boost/regex.hpp>
    namespace boost = cppcms_boost;
#endif


namespace cppcms {
namespace locale {

bool num_format::use_parent(std::ios_base &ios) const
{
	return (ext_flags(ios) & flags::display_flags_mask) == flags::posix;
}

num_format::iter_type num_format::put_value(num_format::iter_type out,std::ios_base &ios,char fill,long double value) const
{
	uint64_t flag = ext_flags(ios) & flags::display_flags_mask;
	if(flag <= flags::date || flag <=flags::strftime) {
		std::string format;
		switch(flag) {
		case flags::date: format="%x"; break;
		case flags::time: format="%X"; break;
		case flags::datetime: format="%c"; break;
		case flags::strftime: format=ext_pattern<char>(ios,flags::datetime_pattern); break;
		}
		std::string timezone = ext_pattern<char>(ios,flags::time_zone_id); 

		std::tm tm;
		time_t time=static_cast<time_t>(value);
		if(!timezone.empty()) {
			boost::cmatch m;
			static boost::regex r("^([Gg][Mm][Tt]|[Uu][Tt][Cc])([\\+\\-]?(\\d+))(:(\\d+))?$");
			if(boost::regex_match(timezone.c_str(),m,r)) {
				time += 3600 * atoi(std::string(m[2]).c_str()) 
					+ 60 * atoi(std::string(m[5]).c_str());
			}
			#ifdef HAVE_GMTIME_R
				gmtime_r(&time,&tm);
			#elif defined(CPPCMS_WIN_NATIVE)
				tm=*gmtime(&time); // TLS
			#else
			#	error "No gmtime_r and no thread safe gmtime is given"
			#endif
		}
		else {
			#ifdef HAVE_LOCALTIME_R
				localtime_r(&time,&tm);
			#elif defined(CPPCMS_WIN_NATIVE)
				tm=*localtime(&time); // TLS
			#else
			#	error "No localtime_r and no thread safe localtime is given"
			#endif
		}
		return std::use_facet<std::time_put<char> >(ios.getloc()).put(out,ios,fill,&tm,format.data(),format.data()+format.size());
	}
	else if(flag == flags::currency) {
		bool intl = (ext_flags(ios) & flags::currency_flags_mask)==flags::currency_iso;
		int digits;
		std::locale loc(ios.getloc());
		if(intl)
			digits=std::use_facet<std::moneypunct<char,true> >(loc).frac_digits();
		else
			digits=std::use_facet<std::moneypunct<char,false> >(loc).frac_digits();
		while(digits > 0)
			value*=10;
		std::ios_base::fmtflags f=ios.flags();
		ios.flags(f | std::ios_base::showbase);
		out = std::use_facet<std::money_put<char> >(loc).put(out,intl,ios,fill,value);
		ios.flags(f);
		return out;
	}
	return out;
}


} // locale
} // cppcms
