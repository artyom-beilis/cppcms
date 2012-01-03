///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/http_content_type.h>
#include "http_protocol.h"
#include <algorithm>
#include <string.h>

namespace cppcms { namespace http {
	struct content_type::data {
		std::string type;
		std::string subtype;
		std::string media_type;
		std::map<std::string,std::string> parameters;
	};
	content_type::content_type()
	{
	}
	content_type::content_type(content_type const &other) :
		d(other.d)
	{
	}
	content_type::~content_type()
	{
	}
	content_type const &content_type::operator=(content_type const &other)
	{
		d=other.d;
		return *this;
	}
	std::string content_type::type() const
	{
		if(d)
			return d->type;
		return std::string();
	}
	std::string content_type::subtype() const
	{
		if(d)
			return d->subtype;
		return std::string();
	}
	std::string content_type::media_type() const
	{
		if(d)
			return d->media_type;
		return std::string();
	}
	std::string content_type::charset() const
	{
		return parameter_by_key("charset");
	}
	std::map<std::string,std::string> content_type::parameters() const
	{
		if(d) {
			return d->parameters;
		}
		return std::map<std::string,std::string>();
	}
	bool content_type::parameter_is_set(std::string const &key) const
	{
		if(!d)
			return false;
		if(d->parameters.find(key)==d->parameters.end())
			return false;
		return true;
	}
	std::string content_type::parameter_by_key(std::string const &key) const
	{
		if(!d)
			return std::string();
		std::map<std::string,std::string>::const_iterator p;
		p = d->parameters.find(key);
		if( p == d->parameters.end())
			return std::string();
		return p->second;
	}
	void content_type::parse(char const *begin,char const *end)
	{
		begin = protocol::skip_ws(begin,end);	
		if(begin == end)
			return;
		char const *type_end = protocol::tocken(begin,end);
		if(type_end == begin)
			return;
		std::string type(begin,type_end);
		begin = type_end;
		if(begin == end || *begin++ != '/')
			return;
		type_end = protocol::tocken(begin,end);
		if(type_end == begin)
			return;
		std::string subtype(begin,type_end);
		begin = type_end;
		std::transform(type.begin(),type.end(),type.begin(),protocol::ascii_to_lower);
		std::transform(subtype.begin(),subtype.end(),subtype.begin(),protocol::ascii_to_lower);
		d->type = type;
		d->subtype = subtype;
		d->media_type = type + "/" + subtype;
		while(begin!=end) {
			begin = protocol::skip_ws(begin,end);
			if(begin == end || *begin++ != ';' || (begin=protocol::skip_ws(begin,end))==end)
				return;
			char const *param_end = protocol::tocken(begin,end);
			if(param_end == begin)
				return;
			std::string param(begin,param_end);
			std::transform(param.begin(),param.end(),param.begin(),protocol::ascii_to_lower);
			begin = protocol::skip_ws(param_end,end);
			if(begin==end || *begin++!='=')
				return;
			begin = protocol::skip_ws(begin,end);
			if(begin==end)
				return;
			std::string value;
			if(*begin == '"') {
				char const *tmp = begin;
				value = protocol::unquote(tmp,end);
				if(tmp == begin)
					return;
				begin = tmp;
			}
			else {
				char const *tmp = protocol::tocken(begin,end);
				if(tmp == begin)
					return;
				value.assign(begin,tmp);
				begin = tmp;
			}
			d->parameters.insert(std::make_pair(param,value));
		}
	}
	content_type::content_type(char const *begin,char const *end) :
		d(new content_type::data())
	{
		parse(begin,end);
	}
	content_type::content_type(char const *s) :
		d(new content_type::data())
	{
		char const *begin = s;
		char const *end = s + strlen(s);
		parse(begin,end);
	}
	content_type::content_type(std::string const &ct) :
		d(new content_type::data())
	{
		char const *begin = ct.c_str();
		char const *end = begin + ct.size();
		parse(begin,end);
	}
} // http
} // cppcms
