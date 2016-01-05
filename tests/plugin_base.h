///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2016  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TESTS_PLUGIN_BASE_H
#define TESTS_PLUGIN_BASE_H
struct bar_base {
 	virtual char const *msg() = 0;
	virtual ~bar_base() {}
};
#endif
