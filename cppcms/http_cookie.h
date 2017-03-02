///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HTTP_COOKIE_H
#define CPPCMS_HTTP_COOKIE_H

#include <cppcms/defs.h>
#include <booster/copy_ptr.h>

#include <string>
#include <iostream>
#include <cppcms/cstdint.h>
namespace cppcms { namespace http {

class cookie;
std::ostream CPPCMS_API &operator<<(std::ostream &,cookie const &);

///
/// \brief Class that represents single HTTP Cookie
/// Generally used in context of http::request and http::response
///

class CPPCMS_API cookie {
public:
	///
	/// Cookie's Name
	///
	std::string name() const;

	///
	/// Cookie's value
	///
	std::string value() const;
	///
	/// Cookie's path
	///
	std::string path() const;

	///
	/// Cookie's domain
	///
	std::string domain() const;
	///
	/// Cookie's comment
	///
	std::string comment() const;

	///
	/// Check if the cookie is transferred over secure connection only
	///
	bool secure() const;

	///
	/// Set cookie's name
	///
	void name(std::string n);

	///
	/// Set cookie's value
	///
	void value(std::string v);

	///
	/// Set cookie's path
	///
	void path(std::string p);

	///
	/// Set cookie's domain
	///
	void domain(std::string);
	///
	/// Set cookie's comment
	///
	void comment(std::string);

	///
	/// Set expiration date/time
	///
	void expires(time_t when);
	///
	/// Returns expires timestamp for the cookie, if not set returns 0
	///
	/// \ver{v1_2}
	time_t expires() const;

	/// 
	/// returns true if expires(time_t when) was called and expiration was set,
	/// if browser_age() is called it is reset to false
	///
	/// \ver{v1_2}
	bool expires_defined() const;
	///
	/// Set max cookie's age
	///
	void max_age(unsigned age);
	///
	/// Get max cookie's age, returns 0 if not set
	///
	/// \ver{v1_2}
	unsigned max_age() const;
	/// 
	/// returns true if max(unsigned age) was called and max_age was set,
	/// if browser_age() is called it is reset to false
	///
	/// \ver{v1_2}
	bool max_age_defined() const;
	///
	/// Set age according to browser's session (i.e. no Max-Age)
	///
	void browser_age();

	///
	/// Set secure property on the cookies
	///
	void secure(bool v);

	///
	/// Check if cookie is not assigned - empty
	///
	bool empty() const;

	cookie();
	~cookie();
	cookie(cookie const &);
	cookie const &operator=(cookie const &);

	///
	/// Create cookie with name and value, age - browser, rest properties undefined.
	///
	cookie(std::string name,std::string value);
	///
	/// Create cookies with name, value and max-age, rest properties undefined.
	///
	cookie(std::string name,std::string value,unsigned age);
	///
	/// Create cookie with name, value, max-age, path, domain and command 
	///
	cookie(std::string name,std::string value,unsigned age,std::string path,std::string domain = std::string(),std::string comment=std::string());
	///
	/// Create cookie with name, value, path, domain and comment, age - browser.
	cookie(std::string name,std::string value,std::string path,std::string domain=std::string(),std::string comment=std::string());

private:
	friend std::ostream &operator<<(std::ostream &,cookie const &);

	void write(std::ostream &) const;
	// for future use
	struct _data;
	booster::copy_ptr<_data> d;

	// real members
	std::string name_;
	std::string value_;
	std::string path_;
	std::string domain_;
	std::string comment_;

	unsigned max_age_;

	uint32_t secure_	: 1;
	uint32_t has_age_	: 1;
	uint32_t has_expiration_: 1;
	CPPCMS_UNUSED_MEMBER uint32_t reserved_	: 29;
};




} } //::cppcms::http


#endif
