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
#include <cppcms/base_view.h>
#include <cppcms/util.h>
#include <cppcms/cppcms_error.h>

#include <vector>
#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/format.hpp>
#else // Internal Boost
#   include <cppcms_boost/format.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms {

struct base_view::data {
	std::ostream *out;
};

base_view::base_view(std::ostream &out) :
	d(new data)
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
