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
#define CPPCMS_SOURCE
#include <cppcms/defs.h>
#include "send_timeout.h"
#ifndef CPPCMS_WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

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

