///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
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
