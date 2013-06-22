///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/http_cookie.h>
#include "http_protocol.h"
#include <cppcms/cppcms_error.h>
#include <booster/posix_time.h>

#include <sstream>
#include <locale>

namespace cppcms { namespace http {

struct cookie::_data { time_t expires; };
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

void cookie::expires(time_t when)
{
	if(!d.get()) {
		d.reset(new _data());
	}
	has_expiration_=1;
	d->expires = when;
}
void cookie::max_age(unsigned age)
{ 
	has_age_=1;
	max_age_=age;
}
void cookie::browser_age()
{
	has_age_=0;
	has_expiration_=0;
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
	if(has_age_ || has_expiration_) {
		std::locale l=std::locale::classic();
		std::stringstream ss;
		ss.imbue(l);
		if(has_age_)
			ss<<"; Max-Age="<<max_age_;

		if(has_expiration_ && d.get()) {
			ss<<"; Expires=";
			
			std::tm splitted = booster::ptime::universal_time(booster::ptime(d->expires));
			static char const format[]="%a, %d %b %Y %H:%M:%S GMT";
			char const *b=format;
			char const *e=b+sizeof(format)-1;
			std::use_facet<std::time_put<char> >(l).put(ss,ss,' ',&splitted,b,e);
		}
		out << ss.rdbuf();
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
	name_(name), value_(value), secure_(0), has_age_(0), has_expiration_(0)
{
}

cookie::cookie(std::string name,std::string value,unsigned age) :
	name_(name), value_(value), max_age_(age), secure_(0), has_age_(1), has_expiration_(0)
{
}
cookie::cookie(std::string name,std::string value,unsigned age,std::string path,std::string domain,std::string comment) :
	name_(name), value_(value), path_(path),domain_(domain),comment_(comment),max_age_(age), secure_(0), has_age_(1), has_expiration_(0)
{
}

cookie::cookie(std::string name,std::string value,std::string path,std::string domain,std::string comment) :
	name_(name), value_(value), path_(path),domain_(domain),comment_(comment), secure_(0), has_age_(0), has_expiration_(0)
{
}

cookie::cookie(cookie const &other) :
	d(other.d),
	name_(other.name_),
	value_(other.value_),
	path_(other.path_),
	domain_(other.domain_),
	comment_(other.comment_),
	max_age_(other.max_age_),
	secure_(other.secure_),
	has_age_(other.has_age_),
	has_expiration_(other.has_expiration_)
{
}

cookie const &cookie::operator=(cookie const &other)
{
	d=other.d;
	name_=other.name_;
	value_=other.value_;
	path_=other.path_;
	domain_=other.domain_;
	comment_=other.comment_;
	max_age_=other.max_age_;
	secure_=other.secure_;
	has_age_=other.has_age_;
	has_expiration_ = other.has_expiration_;
	return *this;
}

cookie::cookie() : secure_(0), has_age_(0), has_expiration_(0) {}
cookie::~cookie() {}


} } // cppcms::http

