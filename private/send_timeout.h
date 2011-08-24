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
#ifndef CPPCMS_IMPL_SEND_TIMEOUT_H
#define CPPCMS_IMPL_SEND_TIMEOUT_H
#include <booster/aio/stream_socket.h>
#include <booster/system_error.h>

namespace cppcms {
namespace impl {

void set_send_timeout(booster::aio::stream_socket &sock,int seconds,booster::system::error_code &e);
void set_send_timeout(booster::aio::stream_socket &sock,int seconds);

} // impl
} // cppcms

#endif
