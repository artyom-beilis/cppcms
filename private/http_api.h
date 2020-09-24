///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_HTTP_API_H
#define CPPCMS_IMPL_HTTP_API_H

#include <cppcms/defs.h>
#include <string>
#include <booster/auto_ptr_inc.h>

namespace cppcms {
	class service;
namespace impl {
namespace cgi {
	class acceptor;
	std::unique_ptr<acceptor> http_api_factory(cppcms::service &srv,std::string ip,int port,int backlog);
} // cgi
} // impl
} // cppcms

#endif

