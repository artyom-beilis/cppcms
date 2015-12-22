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

#include <iostream>
#include "http_file_buffer.h"

namespace cppcms {
namespace http {


struct file::impl_data { 
	impl::file_buffer fb;
	std::istream in;
	std::ostream out;
	impl_data() :
		in(&fb),
		out(&fb)
	{
	}
};

std::string file::name() const
{
	return name_;
}

std::string file::mime() const
{
	return mime_;
}

bool file::has_mime() const
{
	return !mime_.empty();
}


std::string file::filename() const
{
	return filename_;
}
long long file::size() 
{
	return d->fb.size();
}

std::istream &file::data()
{
	return d->in;
}


std::ostream &file::write_data()
{
	return d->out;
}

void file::make_permanent()
{
	file_temporary_ = 0;
}

void file::output_file(std::string const &name,bool is_temporary)
{
	d->fb.name(name);
	if(!is_temporary) {
		if(d->fb.to_file()!=0) {
			throw cppcms_error("Failed to write to file " + name);
		}
	}
	file_specified_ = 1;
	file_temporary_ = is_temporary ? 1:0;

}

void file::copy_stream(std::istream &in,std::ostream &out)
{
	out << in.rdbuf();
}

void file::save_to(std::string const &filename)
{
	d->in.clear(); 
	d->in.seekg(0);
	d->fb.pubsync();

	if(d->fb.in_memory()) {
		save_by_copy(filename,d->in);
		return;
	}
	#ifdef CPPCMS_WIN32
		d->fb.close();
		/// we can't move opened file on windows as it would be locked 
		if(booster::nowide::rename(d->fb.name().c_str(),filename.c_str())!=0) {
			booster::nowide::ifstream tmp(d->fb.name().c_str(),std::ios_base::binary | std::ios_base::in);
			if(!tmp) {
				throw cppcms_error("Failed to reopen file");
			}
			save_by_copy(filename,tmp);
			tmp.close();
			booster::nowide::remove(d->fb.name().c_str());
		}
	#else
		if(booster::nowide::rename(d->fb.name().c_str(),filename.c_str())!=0) {
			save_by_copy(filename,d->in);
			booster::nowide::remove(d->fb.name().c_str());
		}
		d->fb.close();
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
	d->fb.set_limit(size);
}

void file::set_temporary_directory(std::string const &dir)
{
	d->fb.temp_dir(dir);
}

file::file() :
	removed_(0),
	file_specified_(0),
	file_temporary_(1),
	d(new impl_data())
{
}


int file::close()
{
	if(!d->fb.in_memory() && !removed_) {
		int r = d->fb.close();
		if(file_temporary_ && !d->fb.name().empty()) {
			booster::nowide::remove(d->fb.name().c_str());
			removed_ = 1;
		}
		return r;
	}
	else 
		return d->fb.close();
}

file::~file()
{
	close();
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
