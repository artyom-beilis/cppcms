///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2015  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_HTTP_FILE_BUFFER_H
#define CPPCMS_IMPL_HTTP_FILE_BUFFER_H
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#include <cppcms/config.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef CPPCMS_HAVE_FSEEKO
// nothing
#elif defined(CPPCMS_HAVE_FSEEKI64)
#define fseeko(f,o,w) _fseeki64(f,o,w)
#else
#define fseeko(f,o,w) fseek(f,o,w)
#endif

#include <booster/nowide/cstdio.h>
#include <cppcms/urandom.h>
#include "tohex.h"
#include <streambuf>
#include <vector>
#include <stdlib.h>

namespace cppcms {
namespace http {
namespace impl {

//
// Properties:
// 
// output - write only no seek 
// input - independent seek of write
// if fime size is under certain limit - goes to memory
// tracs full size via size() member function
//

class file_buffer : public std::streambuf {
public:
	static const size_t buffer_size = 1024;
	file_buffer(size_t mlimit = 0) :
		in_memory_(true),
		f_(0),
		limit_(mlimit),
		file_size_(0),
		read_offset_(0),
		closed_(false)
	{
		setp(0,0);
		setg(0,0,0);
	}
	~file_buffer()
	{
		if(f_)
			fclose(f_);
	}

	bool in_memory()
	{
		return in_memory_;
	}

	void set_limit(size_t mlimit)
	{
		if(in_memory_ && limit_ < mlimit) {
			limit_ = mlimit;
		}
	}

	void temp_dir(std::string const &tdir)
	{
		if(!in_memory_)
			throw booster::logic_error("Can't update temporary dir for open file");
		temp_dir_ = tdir;
	}
	
	std::string name()
	{
		return name_;
	}
	void name(std::string const &n)
	{
		if(!in_memory_)
			throw booster::logic_error("File name updated on open file");
		name_ = n;
	}


	int close()
	{
		if(closed_)
			return 0;
		if(sync() < 0)
			return -1;
		if(f_) {
			if(fclose(f_)!=0)  {
				f_ = 0;
				return -1;
			}
			f_ = 0;
		}
		setp(0,0);
		setg(0,0,0);
		clear(input_);
		clear(output_);
		clear(data_);
		closed_ = true;
		return 0;
	}
#ifdef DEBUG_FILE_BUFFER
	void status(char const *name = "")
	{
		printf("------------ %s------------\n",name);
		printf("  in_memory=%d\n",in_memory_);
		printf("  limit = %d\n",int(limit_));
		printf("  pbuf = {%d|%d} of %d\n",int(pptr()-pbase()),int(epptr()-pptr()),int(epptr()-pbase()));
		printf("  gbuf = {%d|%d} of %d\n",int(gptr()-eback()),int(egptr()-gptr()),int(egptr()-eback()));
		printf("  file_size   = %5lld\n",file_size_);
		printf("  read_offset = %5lld\n",read_offset_);
		printf("\n");
	}
#endif
	long long size()
	{
		return file_size_ + (pptr() - pbase());
	}
	int to_file()
	{
		if(!in_memory_)
			return 0;
		size_t read_offset =gptr() - eback();
		if(write_buffer() != 0)
			return -1;
		clear(data_);
		output_.resize(buffer_size);
		setp(&output_[0],&output_[0]+buffer_size);
		setg(0,0,0);
		read_offset_ = read_offset;
		in_memory_=false;
		return 0;
	}

private:
	void get_name()
	{
		if(!name_.empty())
			return;
		std::string tmp_dir;
		if(temp_dir_.empty()) {
			char const *tmp=getenv("TEMP");
			if(!tmp)
				tmp=getenv("TMP");
			if(!tmp)
				tmp="/tmp";
			tmp_dir=tmp;
		}
		else {
			tmp_dir = temp_dir_;
		}

		name_ = tmp_dir + "/cppcms_uploads_";
		urandom_device rnd;
		char buf[16];
		char rand[33]={0};
		rnd.generate(buf,16);
		cppcms::impl::tohex(buf,sizeof(buf),rand);
		name_.append(rand);
		name_+=".tmp";
	}

protected:
	std::streampos seekpos(std::streampos off,std::ios_base::openmode mode)
	{
		return seekoff(off,std::ios_base::beg,mode);
	}

	std::streampos seekoff (std::streamoff off, std::ios_base::seekdir way,std::ios_base::openmode mode)
	{
		if(mode & std::ios_base::out) {
			if(off != 0 || way != std::ios_base::cur)
				return -1;
			return size();
		}
		if(in_memory_) {
			size_t rpos;
			size_t stream_size = pptr() - pbase();
			switch(way) {
			case std::ios_base::beg: rpos = off ; break;
			case std::ios_base::cur: rpos = gptr() - eback() + off; break;
			case std::ios_base::end: rpos = stream_size + off; break;
			default: return -1;
			}
			if(rpos > stream_size)
				return -1;
			setg(pbase(),pbase()+rpos,pptr());
			return rpos;
		}
		else {
			if(sync() < 0)
				return -1;
			read_offset_ += gptr() - eback();
			setg(0,0,0);
			long long new_offset;
			switch(way) {
			case std::ios_base::beg: new_offset = off; break;
			case std::ios_base::cur: new_offset = read_offset_ + off; break;
			case std::ios_base::end: new_offset = file_size_ + off; break;
			default: return -1;
			}

			if(new_offset < 0 || new_offset > file_size_) {
				new_offset = file_size_;
				return -1;
			}
			read_offset_ = new_offset;
			return read_offset_;
		}
	}
	int pbackfail(int)
	{
		if(in_memory_)
			return -1;
		if(read_offset_ == 0)
			return -1;

		int by = buffer_size / 2;

		if(read_offset_ < by) 
			by = read_offset_;

		if(seekoff(-by,std::ios_base::cur,std::ios_base::in) < 0)
			return -1;
		if(underflow()  < 0)
			return -1;
		gbump(by - 1);
		return std::char_traits<char>::to_int_type(*gptr());
	}
	int underflow()
	{
		if(in_memory_) {
			size_t read_size = gptr() - eback();
			setg(pbase(),pbase()+read_size,pptr());
		}
		else {
			if(sync() < 0)
				return -1;
			read_offset_ += gptr() - eback();
			if(fseeko(f_,read_offset_,SEEK_SET)!=0)
				return -1;
			input_.resize(buffer_size);
			char *input_data = &input_[0];
			size_t n = fread(input_data,1,buffer_size,f_);
			setg(input_data,input_data,input_data+n);
		}
		if(gptr()==egptr())
			return -1;
		return std::char_traits<char>::to_int_type(*gptr());
	}
	int overflow(int c)
	{
		size_t size = pptr() - pbase();
		if(in_memory_) {
			if(size >= limit_) {
				if(to_file() < 0)
					return -1;
			}
			else {
				size_t read_offset =gptr() - eback();
				size_t new_size = data_.size() * 2;
				if(new_size == 0)
					new_size = 64;
				if(new_size > limit_)
					new_size = limit_;
				data_.resize(new_size);

				setp(&data_[0],&data_[0] + data_.size());
				pbump(size);

				setg(pbase(),pbase() + read_offset,pptr());
			}
		}
		else {
			if(write_buffer()!= 0)
				return -1;
			setp(pbase(),epptr());
		}
		if(c!=EOF) {
			*pptr() = c;
			pbump(1);
		}
		return 0;
	}
	int sync()
	{
		if(in_memory_)
			return 0;
		if(write_buffer()!=0)
			return -1;
		if(fflush(f_)!=0)
			return -1;
		return 0;
	}
	int write_buffer()
	{
		if(closed_)
			return -1;
		if(!f_) {
			get_name();
			f_ = booster::nowide::fopen(name_.c_str(),"w+b");
			if(!f_)
				return -1;
		}
		if(fseek(f_,0,SEEK_END) !=0)
			return -1;
		size_t size = pptr()-pbase();
		if(size != 0 && fwrite(pbase(),1,size,f_)!=size)
			return -1;
		file_size_ += size;
		setp(pbase(),epptr());
		return 0;
	}
private:
	void clear(std::vector<char> &v)
	{
		std::vector<char> tmp;
		tmp.swap(v);
	}
	bool in_memory_;
	FILE *f_;
	size_t limit_;
	long long file_size_;
	long long read_offset_;
	std::vector<char> input_;
	std::vector<char> output_;
	std::vector<char> data_;
	std::string temp_dir_;
	std::string name_;
	bool closed_;
};

} // impl
} // http
} // cppcms
#endif
