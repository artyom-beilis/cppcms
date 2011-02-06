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
#include <cppcms/base_content.h>
#include <cppcms/cppcms_error.h>

namespace cppcms {

struct base_content::_data {};

base_content::base_content() : 
	app_(0)
{
}

base_content::base_content(base_content const &other) :
	d(other.d),
	app_(other.app_)
{
}

base_content::~base_content()
{
}

base_content const &base_content::operator=(base_content const &other)
{
	d = other.d; 
	app_ = other.app_;
	return *this;
}

void base_content::app(application &app)
{
	app_ = &app;
}

application &base_content::app()
{
	if(!app_) {
		throw cppcms_error("Attempt to access to application that wasn't set");
	}
	return *app_;
}

void base_content::reset_app()
{
	app_ = 0;
}


}// cppcms
