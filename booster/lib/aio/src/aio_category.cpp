//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/aio/aio_category.h>

namespace booster {
namespace aio{ 
namespace aio_error {
	char const *category::name() const
	{
		return "aio::";
	}
	std::string category::message(int cat) const
	{
		switch(cat) {
		case ok: return "ok";
		case canceled: return "canceled";
		case select_failed: return "connection hang-up or invalid discriptor tested";
		case eof: return "eof";
		case invalid_endpoint: return "invalid endpoint";
		case no_service_provided: return "no io_service provided";
		case prefork_not_enabled: return "prefork acceptor is not enabled";
		default:
			return "unknown";
		}
	}
	
	category const &get_category()
	{
		static const category cat;
		return cat;
	}
	
} // aio_error

} // aio
} // booster
