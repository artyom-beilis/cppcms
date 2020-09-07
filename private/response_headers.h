///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2020  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_PRIVATE_RESPONSE_HEADERS_H
#define CPPCMS_PRIVATE_RESPONSE_HEADERS_H
#include "http_protocol.h"
#include <list>

namespace cppcms {
namespace impl {

struct icompare_type {
	bool operator()(std::string const &left,std::string const &right) const
	{
		return http::protocol::compare(left,right) < 0;
	}
};

class response_headers {
public:
	class string_buffer_wrapper {
	public:
		string_buffer_wrapper(size_t n = 0)
		{
			if(n!=0)
				string_.reserve(n);
		}

		std::string &data()
		{
			return string_;
		}
		
		string_buffer_wrapper &operator<<(std::string const &s)
		{
			string_ += s;
			return *this;
		}
		string_buffer_wrapper &operator<<(char const *s)
		{
			string_ += s;
			return *this;
		}
		string_buffer_wrapper &operator<<(char c)
		{
			string_ += c;
			return *this;
		}
	private:
		std::string string_;
	};
	size_t estimate_size() const
	{
		size_t n=32; // final crfl + http header
		for(auto const &h : headers_) {
			n+=h.first.size() + h.second.size() + 4; // ": " "\r\n"
		}
		for(auto const &h : added_headers_) {
			n+=h.size() + 2; // add crlf
		}
		return n;
	}
	template<typename IODevice>
	void format_http_headers(IODevice &io,char const *http_version,bool complete) const
	{
		io << "HTTP/" << http_version << " ";;
		auto status_ptr = headers_.find("Status");
		if(status_ptr == headers_.end()) {
			io << "200 Ok";
		}
		else {
			io << status_ptr->second;
		}
		io << "\r\n";
		for(auto p=std::begin(headers_),e=std::end(headers_);p!=e;++p) {
			if(p!=status_ptr)
				io << p->first << ": " << p->second << "\r\n";
		}
		for(auto const &h: added_headers_) {
			io <<  h << "\r\n" ;
		}
		if(complete)
			io << "\r\n";
	}
	template<typename IODevice>
	void format_cgi_headers(IODevice &io,bool complete) const
	{
		for(auto p=std::begin(headers_),e=std::end(headers_);p!=e;++p) {
			io << p->first << ": " << p->second << "\r\n";
		}
		for(auto const &h: added_headers_) {
			io <<  h << "\r\n" ;
		}
		if(complete)
			io << "\r\n";
	}
	void set_header(std::string const &name,std::string const &value)
	{
		if(value.empty())
			headers_.erase(name);
		else
			headers_[name]=value;
	}
	std::string const &get_header(std::string const &name)
	{
		headers_type::const_iterator p=headers_.find(name);
		if(p!=headers_.end())
			return p->second;
		return empty_;
	}
	void erase_header(std::string const &name)
	{
		headers_.erase(name);
	}

	void add_header(std::string &&header)
	{
		added_headers_.push_back(std::move(header));
	}
	void add_header(std::string const &name,std::string const &value)
	{
		if(http::protocol::compare(name,"status")==0 || http::protocol::compare(name,"content-length")==0) {
			set_header(name,value);
			return;
		}
		std::string h;
		h.reserve(name.size() + value.size() + 3);
		h+=name;
		h+=": ";
		h+=value;
		added_headers_.push_back(std::move(h));
	}

private:
	typedef std::map<std::string,std::string,icompare_type> headers_type;
	headers_type headers_;
	std::list<std::string> added_headers_;
	std::string empty_;
};

} } // cppcms//http//impl

#endif // CPPCMS_PRIVATE_RESPONSE_HEADERS_H
