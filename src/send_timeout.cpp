///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/defs.h>
#ifndef CPPCMS_WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif
#include "send_timeout.h"

namespace cppcms {
namespace impl {

#ifndef CPPCMS_WIN32
void set_send_timeout(booster::aio::stream_socket &sock,int seconds,booster::system::error_code &e)
{
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	if(setsockopt(sock.native(),SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv)) < 0) {
		e=booster::system::error_code(errno,booster::system::system_category);
	}
}

#else
void set_send_timeout(booster::aio::stream_socket &sock,int seconds,booster::system::error_code &e)
{
	DWORD tv = seconds * 1000; // milliseconds
	if(setsockopt(sock.native(),SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<char *>(&tv),sizeof(tv)) < 0) {
		e=booster::system::error_code(WSAGetLastError(),booster::system::windows_category);
	}
}
#endif

void set_send_timeout(booster::aio::stream_socket &sock,int seconds)
{
	booster::system::error_code e;
	set_send_timeout(sock,seconds,e);
	if(e) {
		throw booster::system::system_error(e);
	}
}


} // impl
} // cppcms

