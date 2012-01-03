///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
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
