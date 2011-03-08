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
#include <cppcms/http_cookie.h>
#include "http_protocol.h"
#include <cppcms/cppcms_error.h>

#include <sstream>
#include <locale>

namespace cppcms { namespace http {

struct cookie::_data { };
bool cookie::empty() const
{
	return name_.empty() && value_.empty(); 
}

std::string cookie::name() const { return name_; }
void cookie::name(std::string v) { name_=v; }
std::string cookie::value() const { return value_; }
void cookie::value(std::string v) { value_=v; }
std::string cookie::path() const { return path_; }
void cookie::path(std::string v) { path_=v; }
std::string cookie::domain() const { return domain_; }
void cookie::domain(std::string v) { domain_=v; }
std::string cookie::comment() const { return comment_; }
void cookie::comment(std::string v) { comment_=v; }
void cookie::max_age(unsigned age)
{ 
	has_age_=1;
	max_age_=age;
}
void cookie::browser_age()
{
	has_age_=0;
}
bool cookie::secure() const { return secure_; }
void cookie::secure(bool secure) { secure_ = secure ? 1: 0; }

void cookie::write(std::ostream &out) const
{
	if(name_.empty())
		throw cppcms_error("Cookie's name is not defined");
	out<<"Set-Cookie:"<<name_<<'=';
	if(value_.empty()) {
		// Nothing to do write
	}
	if(protocol::tocken(value_.begin(),value_.end())==value_.end())
		out<<value_;
	else
		out<<protocol::quote(value_);
	
	if(!comment_.empty())
		out<<"; Comment="<<protocol::quote(comment_);
	if(!domain_.empty())
		out<<"; Domain="<<domain_;
	if(has_age_) {
		std::ostringstream ss;
		ss.imbue(std::locale("C"));
		ss<<max_age_;
		out<<"; Max-Age="<<ss.str();
	}
	if(!path_.empty())
		out<<"; Path="<<path_;
	if(secure_)
		out<<"; Secure";
	out<<"; Version=1";
}

std::ostream &operator<<(std::ostream &out,cookie const &c)
{
	c.write(out);
	return out;
}

cookie::cookie(std::string name,std::string value) :
	name_(name), value_(value), secure_(0), has_age_(0)
{
}

cookie::cookie(std::string name,std::string value,unsigned age) :
	name_(name), value_(value), max_age_(age), secure_(0), has_age_(1)
{
}
cookie::cookie(std::string name,std::string value,unsigned age,std::string path,std::string domain,std::string comment) :
	name_(name), value_(value), path_(path),domain_(domain),comment_(comment),max_age_(age), secure_(0), has_age_(1)
{
}

cookie::cookie(std::string name,std::string value,std::string path,std::string domain,std::string comment) :
	name_(name), value_(value), path_(path),domain_(domain),comment_(comment), secure_(0), has_age_(0)
{
}

cookie::cookie(cookie const &other) :
	name_(other.name_),
	value_(other.value_),
	path_(other.path_),
	domain_(other.domain_),
	comment_(other.comment_),
	max_age_(other.max_age_),
	secure_(other.secure_),
	has_age_(other.has_age_)
{
}

cookie const &cookie::operator=(cookie const &other)
{
	name_=other.name_;
	value_=other.value_;
	path_=other.path_;
	domain_=other.domain_;
	comment_=other.comment_;
	max_age_=other.max_age_;
	secure_=other.secure_;
	has_age_=other.has_age_;
	return *this;
}

cookie::cookie() : secure_(0), has_age_(0) {}
cookie::~cookie() {}


} } // cppcms::http

