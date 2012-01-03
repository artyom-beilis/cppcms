///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HTTP_CONTENT_TYPE_H
#define CPPCMS_HTTP_CONTENT_TYPE_H

#include <cppcms/defs.h>
#include <booster/shared_ptr.h>
#include <string>
#include <map>

namespace cppcms { namespace http {


///
/// \brief Class that represents parsed Content-Type header, this is immutable
/// class. Once it is created its values does not change.
///
class CPPCMS_API content_type {
public:
	///
	/// The type part of content type, for example, for "text/html; charset=UTF-8" it would be "text" in lower case
	///
	std::string type() const;
	///
	/// The subtype part of content type, for example, for "text/html; charset=UTF-8" it would be "html" in lower case
	///
	std::string subtype() const;
	///
	/// The full media type of content type, for example, for "text/html; charset=UTF-8" it would be "text/html" in lower case
	///
	std::string media_type() const;
	///
	/// Charset parameter, if given, for example, for "text/html; charset=UTF-8" it would be "UTF-8". If charset is not
	/// specified, it would return an empty string
	///
	std::string charset() const;
	///
	/// All parameters, all parameter keys are in lower case, the values are given as is.
	///
	std::map<std::string,std::string> parameters() const;
	///
	/// Get parameter's value by key (should be in lowercase), returns empty string if not set
	///
	std::string parameter_by_key(std::string const &key) const;
	///
	/// Check if the parameter is set using key (should be in lowercase)
	///
	bool parameter_is_set(std::string const &key) const;

	///
	/// Parse content type \a ct and create the class
	///
	content_type(std::string const &ct);
	///
	/// Parse content type \a ct and create the class
	///
	content_type(char const *ct);
	///
	/// Parse content type in range [begin,end) and create the class
	///
	content_type(char const *begin,char const *end);
	///
	/// Empty one... 
	///
	content_type();
	///
	/// Copy constructor
	///
	content_type(content_type const &);
	///
	/// Assignment operator
	///
	content_type const &operator=(content_type const &);
	///
	/// Destructor
	/// 
	~content_type();
private:
	struct data;
	void parse(char const *b,char const *e);
	booster::shared_ptr<data> d;
};


} } //::cppcms::http


#endif
