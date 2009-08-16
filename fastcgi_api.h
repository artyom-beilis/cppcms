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
