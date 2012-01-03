///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/serialization_classes.h>
#include <cppcms/cstdint.h>
#include <string.h>

namespace cppcms {

struct archive::_data {};

archive::archive() :
	ptr_(0),
	mode_(save_to_archive)
{
}

archive::archive(archive const &other) :
	buffer_(other.buffer_),
	ptr_(other.ptr_),
	mode_(other.mode_)
{
}


archive const &archive::operator=(archive const &other)
{
	if(this!=&other) {
		buffer_=other.buffer_;
		ptr_ = other.ptr_;
		mode_=other.mode_;
	}
	return *this;
}

archive::~archive()
{
}

void archive::reserve(size_t n)
{
	buffer_.reserve(n);
}

void archive::write_chunk(void const *begin,size_t len)
{
	uint32_t size  = len;
	buffer_.append(reinterpret_cast<char *>(&size),4);
	buffer_.append(reinterpret_cast<char const *>(begin),len);
}


void archive::read_chunk(void *begin,size_t len)
{
	size_t next = next_chunk_size();
	if(next!=len)
		throw archive_error("Invalid block length");
	ptr_+=4;
	memcpy(begin,buffer_.c_str()+ptr_,len);
	ptr_+=len;
}

size_t archive::next_chunk_size()
{
	uint32_t size=0;
	if(eof())
		throw archive_error("At end of archive");
	if(buffer_.size() - ptr_ < 4) {
		throw archive_error("Invalid archive format");
	}
	memcpy(&size,buffer_.c_str() + ptr_,4);
	if(ptr_ + size < ptr_ || ptr_ + size >=buffer_.size())
		throw archive_error("Invalid archive_format");

	return size;
}

bool archive::eof()
{
	return ptr_ >= buffer_.size();
}


std::string archive::read_chunk_as_string()
{
	size_t size = next_chunk_size();
	std::string result(buffer_.c_str() + ptr_ + 4,size);
	ptr_ +=4+size;
	return result;
}

archive::mode_type archive::mode()
{
	return mode_;
}

void archive::mode(archive::mode_type m)
{
	mode_ = m;
	ptr_ = 0;
}

void archive::reset()
{
	ptr_ = 0;
}

std::string archive::str()
{
	return buffer_;
}

void archive::str(std::string const &str)
{
	buffer_ = str;
	mode_ = load_from_archive;
	ptr_ = 0;
}




} // cppcms
