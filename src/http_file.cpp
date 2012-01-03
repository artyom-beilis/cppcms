///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/http_file.h>
#include <cppcms/urandom.h>
#include <cppcms/cppcms_error.h>
#include <booster/nowide/cstdio.h>
#include <booster/nowide/fstream.h>
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
		if(size() > static_cast<long long>(size_limit_)) {
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
		file_data_.clear();
		file_data_.seekg(0);
		save_by_copy(filename,file_data_);
		return;
	}
	file_.clear();
	file_.seekg(0);
	file_.sync();
	#ifdef CPPCMS_WIN32
		file_.close();
		/// we can't move opened file on windows as it would be locked 
		if(booster::nowide::rename(tmp_file_name_.c_str(),filename.c_str())!=0) {
			file_.open(tmp_file_name_.c_str(),std::ios_base::binary | std::ios_base::in | std::ios_base::out);
			if(!file_) {
				throw cppcms_error("Failed to reopen file");
			}
			save_by_copy(filename,file_);
			file_.close();
			booster::nowide::remove(tmp_file_name_.c_str());
		}
	#else
		if(booster::nowide::rename(tmp_file_name_.c_str(),filename.c_str())!=0) {
			save_by_copy(filename,file_);
			booster::nowide::remove(tmp_file_name_.c_str());
		}
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
	f << std::flush;
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
	std::string tmp_dir;
	if(temporary_dir_.empty()) {
		char const *tmp=getenv("TEMP");
		if(!tmp)
			tmp=getenv("TMP");
		if(!tmp)
			tmp="/tmp";
		tmp_dir=tmp;
	}
	else {
		tmp_dir = temporary_dir_;
	}

	tmp_file_name_ = tmp_dir + "/cppcms_uploads_";
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
			booster::nowide::remove(tmp_file_name_.c_str());
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
