//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/config.h>
#ifndef BOOSTER_WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <sstream>
#endif
#include <booster/aio/endpoint.h>
#include <booster/aio/aio_config.h>
#include <booster/system_error.h>
#include <booster/aio/aio_category.h>
#include <booster/aio/types.h>
#include <string.h>




namespace booster {
namespace aio {

struct endpoint::data {
	int size;
	union {
		struct sockaddr_in in;
#ifndef BOOSTER_AIO_NO_PF_INET6
		struct sockaddr_in6 in6;
#endif
#ifndef BOOSTER_AIO_NO_PF_UNIX
		struct sockaddr_un un;
#endif
		struct sockaddr sa;
		struct sockaddr_storage storage;
		char filler[256];
	} sa;

	data() : size(0)
	{
		::memset(&sa,0,sizeof(sa));
	}
};


void endpoint::throw_invalid() const
{
	throw system::system_error(system::error_code(aio_error::invalid_endpoint,aio_error_cat));
}


endpoint::endpoint() : d(new data())
{
}
endpoint::endpoint(endpoint const &other) : d(other.d) {}

endpoint::~endpoint()
{
}

endpoint const &endpoint::operator=(endpoint const &other) 
{
	d=other.d;
	return *this; 
}

endpoint::endpoint(std::string const &ip_in,int port_in) :
	d(new data())
{
	ip(ip_in);
	port(port_in);
}

void endpoint::ip(std::string const &ip_in)
{
	if(ip_in.find('.')!=std::string::npos) {
		struct in_addr inaddr;
#ifndef BOOSTER_WIN32
		if(::inet_pton(AF_INET,ip_in.c_str(),&inaddr)==0)
			throw_invalid();
#else
		unsigned addr = inet_addr(ip_in.c_str());
		if(addr==INADDR_NONE)
			throw_invalid();
		else
			memcpy(&inaddr,&addr,sizeof(inaddr));
#endif
		d->size=sizeof(struct sockaddr_in);
		d->sa.in.sin_family=AF_INET;
		d->sa.in.sin_addr=inaddr;
	}
#ifndef BOOSTER_AIO_NO_PF_INET6
	else if(ip_in.find(':')!=std::string::npos)  {
		struct in6_addr in6addr;
		if(::inet_pton(AF_INET6,ip_in.c_str(),&in6addr)==0)
			throw_invalid();
		d->size=sizeof(struct sockaddr_in6);
		d->sa.in6.sin6_family=AF_INET6;
		d->sa.in6.sin6_addr=in6addr;
	}
#endif
	else {
		throw_invalid();
	}
}

void endpoint::port(int port_no)
{
	if(port_no > 0xFFFF || port_no < 0)
		throw_invalid();
	switch(family()) {
	case pf_inet: d->sa.in.sin_port = htons(port_no); return;
#ifndef BOOSTER_AIO_NO_PF_INET6
	case pf_inet6: d->sa.in6.sin6_port = htons(port_no); return;
#endif
	default:
		throw_invalid();
	}
}

family_type endpoint::family() const
{
	if(d->size < 2)
		throw_invalid();
	switch(d->sa.sa.sa_family) {
	case AF_INET:
		return pf_inet;
#ifndef BOOSTER_AIO_NO_PF_INET6
	case AF_INET6:
		return pf_inet6;
#endif
	case AF_UNIX:
		return pf_unix;
	default:
		throw_invalid();
	}
}


int endpoint::port() const
{
	switch(family()) {
	case pf_inet:
		return ntohs(d->sa.in.sin_port);
#ifndef BOOSTER_AIO_NO_PF_INET6
	case pf_inet6:
		return ntohs(d->sa.in6.sin6_port);
#endif
	default:
		throw_invalid();
	}
}

std::string endpoint::ip() const
{
	switch(family()) {
	case pf_inet:
		{
#ifndef BOOSTER_WIN32
			char buf[INET_ADDRSTRLEN+1] = {0};
			char const *res = ::inet_ntop(AF_INET,&d->sa.in.sin_addr,buf,sizeof(buf));
			if(res)
				return std::string(res);
			throw_invalid();
#else
			std::ostringstream tmp;
			tmp.imbue(std::locale::classic());
			char const *p = reinterpret_cast<char const *>(&d->sa.in.sin_addr);
			tmp << int(p[0]) <<"." << int(p[1]) <<"."<<int(p[2]) <<"." << int(p[2]);
			return tmp.str();
#endif
		}
		break;
#ifndef BOOSTER_AIO_NO_PF_INET6
	case pf_inet6:
		{
			char buf[INET6_ADDRSTRLEN+1] = {0};
			char const *res = ::inet_ntop(AF_INET6,&d->sa.in6.sin6_addr,buf,sizeof(buf));
			if(res)
				return std::string(res);
			throw_invalid();
		}
		break;
#endif
	default:
		throw_invalid();
	}
}

#ifndef BOOSTER_AIO_NO_PF_UNIX

endpoint::endpoint(std::string const &path_in) : d(new data())
{
	path(path_in);
}

std::string endpoint::path() const
{
	if(family()!=pf_unix)
		throw_invalid();
	return d->sa.un.sun_path;
}

void endpoint::path(std::string const &local_socket)
{
	if(local_socket.size() + 1 >  sizeof(d->sa.un.sun_path))
		throw_invalid();
	d->size = sizeof(d->sa.un);
	::strncpy(d->sa.un.sun_path,local_socket.c_str(),sizeof(d->sa.un.sun_path) - 1);
	d->sa.un.sun_path[sizeof(d->sa.un.sun_path) - 1]=0;
	d->sa.un.sun_family = AF_UNIX;
}

#endif

void endpoint::raw(sockaddr const *p,int size)
{
	if(size > int(sizeof(d->sa)))
		throw_invalid();
	d->size=size;
	memcpy(&d->sa.sa,p,size);
}

std::pair<sockaddr const *,int> endpoint::raw() const
{
	std::pair<sockaddr const *,int> res;
	res.first = &d->sa.sa;
	res.second = d->size;
	return res;
}


} // aio
} // booster
