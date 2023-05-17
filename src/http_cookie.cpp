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

struct cookie::_data {};
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
	has_expiration_=1;
	expires_ = when;
}

time_t cookie::expires() const
{
	if(has_expiration_)
		return expires_;
	return 0;
}
bool cookie::expires_defined() const
{
	return has_expiration_ == 1;
}


void cookie::max_age(unsigned age)
{ 
	has_age_=1;
	max_age_=age;
}
bool cookie::max_age_defined() const
{
	return has_age_ == 1;
}
unsigned cookie::max_age() const
{
	if(has_age_)
		return max_age_;
	return 0;
}

void cookie::browser_age()
{
	has_age_=0;
	has_expiration_=0;
}
bool cookie::secure() const { return secure_; }
void cookie::secure(bool secure) { secure_ = secure ? 1: 0; }

bool cookie::httponly() const { return httponly_; }
void cookie::httponly(bool httponly) { httponly_ = httponly ? 1 : 0; }

bool cookie::samesite_none() const { return samesite_none_; }
bool cookie::samesite_lax() const { return samesite_lax_; }
bool cookie::samesite_strict() const { return samesite_strict_; }

void cookie::samesite_none(bool v)
{
	if (v) {
		samesite_none_ = 1;
		samesite_lax_ = 0;
		samesite_strict_ = 0;
	} else {
		samesite_none_ = 0;
	}
}

void cookie::samesite_lax(bool v)
{
	if (v) {
		samesite_none_ = 0;
		samesite_lax_ = 1;
		samesite_strict_ = 0;
	} else {
		samesite_lax_ = 0;
	}
}

void cookie::samesite_strict(bool v)
{
	if (v) {
		samesite_none_ = 0;
		samesite_lax_ = 0;
		samesite_strict_ = 1;
	} else {
		samesite_strict_ = 0;
	}
}

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

		if(has_expiration_) {
			ss<<"; Expires=";
			
			std::tm splitted = booster::ptime::universal_time(booster::ptime(expires_));
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
	if(httponly_)
		out<<"; HttpOnly";
	// The samesite_*_ setters guarantee that only one of the following is set.
	if(samesite_none_)
		out<<"; SameSite=None";
	if(samesite_lax_)
		out<<"; SameSite=Lax";
	if(samesite_strict_)
		out<<"; SameSite=Strict";
	out<<"; Version=1";
}

std::ostream &operator<<(std::ostream &out,cookie const &c)
{
	c.write(out);
	return out;
}

cookie::cookie(std::string name,std::string value) :
	name_(name), value_(value), secure_(0), has_age_(0), has_expiration_(0), httponly_(0), samesite_none_(0), samesite_lax_(0), samesite_strict_(0)
{
}

cookie::cookie(std::string name,std::string value,unsigned age) :
	name_(name), value_(value), max_age_(age), secure_(0), has_age_(1), has_expiration_(0), httponly_(0), samesite_none_(0), samesite_lax_(0), samesite_strict_(0)
{
}
cookie::cookie(std::string name,std::string value,unsigned age,std::string path,std::string domain,std::string comment) :
	name_(name), value_(value), path_(path),domain_(domain),comment_(comment),max_age_(age), secure_(0), has_age_(1), has_expiration_(0), httponly_(0), samesite_none_(0), samesite_lax_(0), samesite_strict_(0)
{
}

cookie::cookie(std::string name,std::string value,std::string path,std::string domain,std::string comment) :
	name_(name), value_(value), path_(path),domain_(domain),comment_(comment), secure_(0), has_age_(0), has_expiration_(0), httponly_(0), samesite_none_(0), samesite_lax_(0), samesite_strict_(0)
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
	expires_(other.expires_),
	secure_(other.secure_),
	has_age_(other.has_age_),
	has_expiration_(other.has_expiration_),
	httponly_(other.httponly_),
	samesite_none_(other.samesite_none_),
	samesite_lax_(other.samesite_lax_),
	samesite_strict_(other.samesite_strict_)
{
}

cookie::cookie(cookie &&other):
	d(std::move(other.d)),
	name_(std::move(other.name_)),
	value_(std::move(other.value_)),
	path_(std::move(other.path_)),
	domain_(std::move(other.domain_)),
	comment_(std::move(other.comment_)),
	max_age_(other.max_age_),
	expires_(other.expires_),
	secure_(other.secure_),
	has_age_(other.has_age_),
	has_expiration_(other.has_expiration_),
	httponly_(other.httponly_),
	samesite_none_(other.samesite_none_),
	samesite_lax_(other.samesite_lax_),
	samesite_strict_(other.samesite_strict_)
	
{
}

cookie &cookie::operator=(cookie &&other)
{
	d=std::move(other.d);
	name_=std::move(other.name_);
	value_=std::move(other.value_);
	path_=std::move(other.path_);
	domain_=std::move(other.domain_);
	comment_=std::move(other.comment_);
	max_age_=other.max_age_;
	expires_=other.expires_;
	secure_=other.secure_;
	has_age_=other.has_age_;
	has_expiration_ = other.has_expiration_;
	httponly_ = other.httponly_;
	samesite_none_ = other.samesite_none_;
	samesite_lax_ = other.samesite_lax_;
	samesite_strict_ = other.samesite_strict_;
	return *this;
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
	expires_=other.expires_;
	secure_=other.secure_;
	has_age_=other.has_age_;
	has_expiration_ = other.has_expiration_;
	httponly_ = other.httponly_;
	samesite_none_ = other.samesite_none_;
	samesite_lax_ = other.samesite_lax_;
	samesite_strict_ = other.samesite_strict_;
	return *this;
}


cookie::cookie() : secure_(0), has_age_(0), has_expiration_(0), httponly_(0), samesite_none_(0), samesite_lax_(0), samesite_strict_(0) {}
cookie::~cookie() {}


} } // cppcms::http

