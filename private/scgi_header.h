#ifndef CPPCMS_SCGI_HEADER_H
#define CPPCMS_SCGI_HEADER_H

#include <map>
#include <string>

namespace cppcms {
	namespace impl {
		CPPCMS_API std::string make_scgi_header(std::map<std::string,std::string> const &env,size_t addon_size);
	}
}

#endif
