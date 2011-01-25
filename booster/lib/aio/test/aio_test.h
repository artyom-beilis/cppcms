//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/config.h>

#ifdef BOOSTER_WIN32
#	include <winsock2.h>
#	include <windows.h>
	#include <io.h>
#	define closefd(x) closesocket(x)
#else
#	include <sys/socket.h>
#	include <sys/types.h>
#	include <arpa/inet.h>
#	include <netinet/in.h>
#	include <sys/ioctl.h>
#	include <unistd.h>
#	define closefd(x) close(x)
#endif
#include <iostream>
#include <sstream>
#include <string.h>

#include <booster/aio/aio_config.h>
#include <booster/aio/types.h>

#define check(ex) TEST(int(ex) >= 0)

int return_code = 0;

#define TEST(X) do { 									\
	if(!(X)) {									\
		std::cerr << "Fail: " << #X << " at " << __LINE__ << std::endl; 	\
		return_code = 1;							\
	} 										\
	/*else {std::cerr << "Pass: " << #X << " at " << __LINE__ << std::endl;}*/	\
} while(0)

#define check(ex) TEST(int(ex) >= 0)

inline void pair(booster::aio::native_type fds[2])
{
	using namespace booster::aio;
	struct sockaddr_in inaddr;
	struct sockaddr addr;
	native_type lst=::socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
	check(lst);
	memset(&inaddr, 0, sizeof(inaddr));
	memset(&addr, 0, sizeof(addr));
	inaddr.sin_family = AF_INET;
	inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	inaddr.sin_port = 0;
	int yes=1;
	check(setsockopt(lst,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(yes)));
	check(bind(lst,(struct sockaddr *)&inaddr,sizeof(inaddr)));
	listen(lst,1);
#ifdef BOOSTER_WIN32
	int
#else
	socklen_t 
#endif
	len=sizeof(inaddr);
	check(getsockname(lst, &addr,&len));
	fds[0]=::socket(AF_INET, SOCK_STREAM,0);
	check(fds[0]);
	check(connect(fds[0],&addr,len));
	fds[1]=accept(lst,0,0);
	check(fds[1]);
	closefd(lst);
}



