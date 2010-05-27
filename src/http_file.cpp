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
#include <cppcms/http_file.h>

namespace cppcms {
namespace http {

std::string file::name() const
{
	return name_;
}

std::string file::mime() const
{
	return mime_;
}

std::string file::filename() const
{
	return filename_;
}
size_t file::size() const
{
	return size_;
}

std::istream &file::_data()
{
	if(saved_in_file_)
		return file_;
	else
		return file_data_;
}


struct file::impl_data {};

file::file()
{
}

file::~file()
{
}



} // http
} // cppcms 
