///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/base_view.h>
#include <cppcms/util.h>
#include <cppcms/cppcms_error.h>

#include <vector>

namespace cppcms {

struct base_view::_data {
	std::ostream *out;
};

base_view::base_view(std::ostream &out) :
	d(new _data)
{
	d->out=&out;
}

std::ostream &base_view::out()
{
	return *d->out;
}

base_view::~base_view()
{
}

void base_view::render()
{
}


}// cppcms
