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
#include <memory>

namespace cppcms {
	class service;
namespace impl {
namespace cgi {
	class acceptor;
	std::auto_ptr<acceptor> http_api_factory(cppcms::service &srv,std::string ip,int port,int backlog);
} // cgi
} // impl
} // cppcms

#endif

