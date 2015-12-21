///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2015  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/http_content_filter.h>
#include <cppcms/http_response.h>
#include "cached_settings.h"

namespace cppcms {
namespace http {


abort_upload::abort_upload(int status_code) :
	cppcms_error(http::response::status_to_string(status_code)),	
	code_(status_code)
{
}

int abort_upload::code() const
{
	return code_;
}

abort_upload::~abort_upload() throw()
{
}

struct content_limits::_data {};

content_limits::content_limits(impl::cached_settings const &s)  :
	content_length_limit_(s.security.content_length_limit * 1024LL),
	file_in_memory_limit_(s.security.file_in_memory_limit),
	multipart_form_data_limit_(s.security.multipart_form_data_limit * 1024LL),
	uploads_path_(s.security.uploads_path)
{
}

content_limits::~content_limits() 
{
}

content_limits::content_limits() :
	content_length_limit_(0),
	file_in_memory_limit_(0),
	multipart_form_data_limit_(0)
{
}

long long content_limits::content_length_limit() const { return content_length_limit_; }
void content_limits::content_length_limit(long long size) { content_length_limit_=size; }

long long content_limits::multipart_form_data_limit() const { return multipart_form_data_limit_; }
void content_limits::multipart_form_data_limit(long long size) { multipart_form_data_limit_=size; }

size_t content_limits::file_in_memory_limit() const { return file_in_memory_limit_; }
void content_limits::file_in_memory_limit(size_t size) { file_in_memory_limit_=size; }

std::string content_limits::uploads_path() const { return uploads_path_; }
void content_limits::uploads_path(std::string const &path) { uploads_path_=path; }


struct basic_content_filter::_data {};
basic_content_filter::basic_content_filter() {}
basic_content_filter::~basic_content_filter() {}
void basic_content_filter::on_end_of_content() {}
void basic_content_filter::on_error() {}

struct multipart_filter::_mp_data{};
multipart_filter::multipart_filter() {}
multipart_filter::~multipart_filter() {}

void multipart_filter::on_new_file(http::file &) {}
void multipart_filter::on_upload_progress(http::file &) {}
void multipart_filter::on_data_ready(http::file &) {}

struct raw_content_filter::_raw_data {};
void raw_content_filter::on_data_chunk(void const *,size_t) {}
raw_content_filter::raw_content_filter() {}
raw_content_filter::~raw_content_filter() {}

} // http
}// cppcms
