///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_FASTCGI_API_H
#define CPPCMS_IMPL_FASTCGI_API_H

#include "defs.h"
#include <string>
#include <memory>

namespace cppcms {
	class service;
namespace impl {
namespace cgi {
	class acceptor;
	std::auto_ptr<acceptor> fastcgi_api_tcp_socket_factory(cppcms::service &srv,std::string ip,int port,int backlog);
#if !defined(CPPCMS_WIN32)
	std::auto_ptr<acceptor> fastcgi_api_unix_socket_factory(cppcms::service &srv,std::string socket,int backlog);
	std::auto_ptr<acceptor> fastcgi_api_unix_socket_factory(cppcms::service &srv,int backlog);
#endif 

} // cgi
} // impl
} // cppcms

#endif
