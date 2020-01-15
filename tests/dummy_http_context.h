///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_DUMMY_HTTP_CONTEXT_H
#define CPPCMS_IMPL_DUMMY_HTTP_CONTEXT_H
#include <cppcms/http_context.h>
#include "dummy_api.h"

class dummy_http_context : public cppcms::http::context {
public:
	dummy_http_context(booster::shared_ptr<dummy_api> conn)
	: cppcms::http::context(conn)
	{
		prepare_request();
	}
};

#endif
