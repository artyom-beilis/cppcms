///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_LOGGING_H
#define CPPCMS_IMPL_LOGGING_H
#include <cppcms/defs.h>

namespace cppcms {
namespace json { class value; }
namespace impl {
CPPCMS_API void setup_logging(json::value const &);
}
}

#endif
