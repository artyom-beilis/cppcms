#define CPPCMS_SOURCE
#include "http_file.h"

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

std::istream &file::data()
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
