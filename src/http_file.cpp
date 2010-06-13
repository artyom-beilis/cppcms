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
#include <cppcms/urandom.h>
#include <cppcms/cppcms_error.h>
#include <stdlib.h>

#include <vector>


namespace cppcms {
namespace http {


struct file::impl_data {};

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
long long file::size() 
{
	if(saved_in_file_) {
		std::streampos now=file_.tellp();
		file_.seekp(0,std::ios_base::end);
		long long size = file_.tellp();
		file_.seekp(now);
		return size;
	}
	else {
		return file_data_.tellp();
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
	if(saved_in_file_) {
		return file_;
	}
	else {
		if(size() > size_limit_) {
			move_to_file();
			return file_;
		}
		else {
			return file_data_;
		}
	}
}

void file::copy_stream(std::istream &in,std::ostream &out)
{
	std::vector<char> v(1024,0);
	while(!in.eof()) {
		in.read(&v.front(),1024);
		out.write(&v.front(),in.gcount());
	}
}

void file::save_to(std::string const &filename)
{
	if(!saved_in_file_) {
		file_data_.seekg(0);
		save_by_copy(filename,file_data_);
		return;
	}
	file_ << std::flush;
	#ifdef CPPCMS_WIN32
	file_.close();
	#endif
	if(rename(tmp_file_name_.c_str(),filename.c_str())<0) {
		#ifdef CPPCMS_WIN32
		file_.open(tmp_file_name_.c_str(),std::ios_base::binary | std::ios_base::in | std::ios_base::out);
		if(!file_) {
			throw cppcms_error("Failed to reopen file");
		}
		#endif
		file_.seekg(0);
		save_by_copy(filename,file_);
		file_.close();
		remove(tmp_file_name_.c_str());
	}
	#ifndef CPPCMS_WIN32
	file_.close();
	#endif
	removed_ = 1;
}

void file::save_by_copy(std::string const &file_name,std::istream &in)
{
	booster::nowide::ofstream f(file_name.c_str(),std::ios_base::binary | std::ios_base::out);
	if(!f) {
		throw cppcms_error("Failed to save open file:"+file_name);
	}
	copy_stream(in,f);
	f.close();

}

void file::set_memory_limit(size_t size)
{
	size_limit_ = size;
}

void file::set_temporary_directory(std::string const &d)
{
	temporary_dir_ = d;
}

void file::move_to_file()
{
	if(temporary_dir_.empty()) {
		char const *tmp=getenv("TEMP");
		if(!tmp)
			tmp=getenv("TMP");
		if(!tmp)
			tmp="/tmp";
		temporary_dir_=tmp;
	}

	tmp_file_name_ = temporary_dir_ + "/cppcms_uploads_";
	urandom_device rnd;
	unsigned char buf[16];
	char rand[33]={0};
	rnd.generate(buf,16);
	for(unsigned i=0;i<16;i++)
		sprintf(rand+i*2,"%02x",buf[i]);
	tmp_file_name_.append(rand);
	tmp_file_name_+=".tmp";
	file_.open(tmp_file_name_.c_str(),
		std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
	if(!file_)
		throw cppcms_error("Failed to create temporary file");
	file_data_.seekg(0);
	copy_stream(file_data_,file_);
	file_data_.str("");
	saved_in_file_ = 1;
}




file::file() :
	size_limit_(1024*128),
	saved_in_file_(0),
	removed_(0)
{
}

file::~file()
{
	if(saved_in_file_ && !removed_) {
		file_.close();
		if(!tmp_file_name_.empty()) {
			remove(tmp_file_name_.c_str());
		}
	}
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
