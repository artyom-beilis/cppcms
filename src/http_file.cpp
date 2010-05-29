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
size_t file::size() 
{
	if(saved_in_file_) {
		return file_.tellp();
	}
	else {
		return file_data_.str().size();
	}
}

std::istream &file::data()
{
	if(saved_in_file_)
		return file_;
	else
		return file_data_;
}


std::ostream &file::write_data()
{
	if(saved_in_file_)
		return file_;
	else
		return file_data_;
}


struct file::impl_data {};

file::file() :
	saved_in_file_(0)
{
}

file::~file()
{
}

void file::filename(std::string const &v)
{
	filename_=v;
}

void file::name(std::string const &v)
{
	name_=v;
}

void file::mime(std::string const &v)
{
	mime_=v;
}


} // http
} // cppcms 
