//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_SOCKET_DETAILS_H
#define BOOSTER_AIO_SOCKET_DETAILS_H

#include <booster/config.h>
#ifndef BOOSTER_WIN32
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#else
#include <winsock2.h>
#include <windows.h>
typedef int socklen_t;
#endif

#include <booster/aio/endpoint.h>
#include <booster/aio/io_service.h>
#include <booster/aio/aio_category.h>
#include <booster/aio/buffer.h>

#include "category.h"

//#define BOOSTER_AIO_FORCE_POLL


namespace booster {
namespace aio {
namespace socket_details {
	inline system::error_code geterror()
	{
		#ifdef BOOSTER_WIN32
		return system::error_code(::WSAGetLastError(),syscat);
		#else
		return system::error_code(errno,syscat);
		#endif
	}
	inline int close_file_descriptor(int fd)
	{
		#ifndef BOOSTER_WIN32
		int res = 0;
		for(;;) {
			res = ::close(fd);
			if(res < 0 && errno==EINTR)
				continue;
			break;
		}
		#else
		int res = ::closesocket(fd);
		#endif
		return res;
	}

} // socket_details

using namespace socket_details;

} // aio
} // booster

#endif

