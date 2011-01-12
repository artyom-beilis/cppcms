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
#include <cppcms/filters.h>
#include <cppcms/base64.h>
#include <cppcms/util.h>
#include <iostream>

namespace cppcms { namespace filters {

	streamable::streamable() 
	{
		set(0,0,0,0);
	}
	streamable::streamable(streamable const &other)
	{
		set(other.ptr_,other.to_stream_,other.to_string_,other.type_);
	}
	streamable::~streamable()
	{
	}
	streamable const &streamable::operator=(streamable const &other)
	{
		if(&other!=this)
			set(other.ptr_,other.to_stream_,other.to_string_,other.type_);
		return *this;
	}
	namespace {
		void ch_to_stream(std::ostream &out,void const *p)
		{
			out<<reinterpret_cast<char const *>(p);
		}
		std::string ch_to_string(std::ios &ios,void const *p)
		{
			return reinterpret_cast<char const *>(p);
		}
		std::string s_to_string(std::ios &ios,void const *p)
		{
			return *reinterpret_cast<std::string const *>(p);
		}
	}
	streamable::streamable(char const *ptr)
	{
		set(ptr,ch_to_stream,ch_to_string,&typeid(char const *));
	}
	template<>
	streamable::streamable(std::string const &str)
	{
		set(&str,to_stream<std::string>,s_to_string,&typeid(std::string));
	}
	std::string streamable::get(std::ios &ios) const
	{
		return to_string_(ios,ptr_);
	}
	void streamable::operator()(std::ostream &out) const
	{
		to_stream_(out,ptr_);
	}
	
	void streamable::set(void const *ptr,to_stream_type tse,to_string_type tst,std::type_info const *type)
	{
		ptr_=ptr;
		to_stream_=tse;
		to_string_=tst;
		type_=type;
	}

	std::type_info const &streamable::type() const
	{
		return *type_;
	}

///////////////////////////////////
	
	struct to_upper::_data {};
	to_upper::to_upper() {}
	to_upper::~to_upper() {}
	to_upper::to_upper(to_upper const &other) : obj_(other.obj_) {}
	to_upper::to_upper(streamable const &obj) : obj_(obj) {}
	to_upper const &to_upper::operator=(to_upper const &other){ obj_ = other.obj_; return *this; }
	void to_upper::operator()(std::ostream &out) const
	{
		std::string tmp =obj_.get(out) ;
		std::locale loc = out.getloc(); 
		out << ::cppcms::locale::to_upper( tmp,loc);
	}

	struct to_lower::_data {};
	to_lower::to_lower() {}
	to_lower::~to_lower() {}
	to_lower::to_lower(to_lower const &other) : obj_(other.obj_) {}
	to_lower::to_lower(streamable const &obj) : obj_(obj) {}
	to_lower const &to_lower::operator=(to_lower const &other){ obj_ = other.obj_; return *this; }
	void to_lower::operator()(std::ostream &out) const
	{
		out << locale::to_lower(obj_.get(out),out.getloc());
	}

	#ifndef CPPCMS_DISABLE_ICU_LOCALIZATION

	struct to_title::_data {};
	to_title::to_title() {}
	to_title::~to_title() {}
	to_title::to_title(to_title const &other) : obj_(other.obj_) {}
	to_title::to_title(streamable const &obj) : obj_(obj) {}
	to_title const &to_title::operator=(to_title const &other){ obj_ = other.obj_; return *this; }
	void to_title::operator()(std::ostream &out) const
	{
		out << locale::to_title(obj_.get(out),out.getloc());
	}

	#endif

	struct escape::_data {};
	escape::escape() {}
	escape::~escape() {}
	escape::escape(escape const &other) : obj_(other.obj_) {}
	escape::escape(streamable const &obj) : obj_(obj) {}
	escape const &escape::operator=(escape const &other){ obj_ = other.obj_; return *this; }
	void escape::operator()(std::ostream &out) const
	{
		out << util::escape(obj_.get(out));
	}

	struct urlencode::_data {};
	urlencode::urlencode() {}
	urlencode::~urlencode() {}
	urlencode::urlencode(urlencode const &other) : obj_(other.obj_) {}
	urlencode::urlencode(streamable const &obj) : obj_(obj) {}
	urlencode const &urlencode::operator=(urlencode const &other){ obj_ = other.obj_; return *this; }
	void urlencode::operator()(std::ostream &out) const
	{
		out << util::urlencode(obj_.get(out));
	}

	struct raw::_data {};
	raw::raw() {}
	raw::~raw() {}
	raw::raw(raw const &other) : obj_(other.obj_) {}
	raw::raw(streamable const &obj) : obj_(obj) {}
	raw const &raw::operator=(raw const &other){ obj_ = other.obj_; return *this; }
	void raw::operator()(std::ostream &out) const
	{
		obj_(out);
	}

	struct base64_urlencode::_data {};
	base64_urlencode::base64_urlencode() {}
	base64_urlencode::~base64_urlencode() {}
	base64_urlencode::base64_urlencode(base64_urlencode const &other) : obj_(other.obj_) {}
	base64_urlencode::base64_urlencode(streamable const &obj) : obj_(obj) {}
	base64_urlencode const &base64_urlencode::operator=(base64_urlencode const &other){ obj_ = other.obj_; return *this; }
	void base64_urlencode::operator()(std::ostream &os) const
	{
		std::string const s=obj_.get(os);
		using namespace cppcms::b64url;
		unsigned char const *begin=reinterpret_cast<unsigned char const *>(s.c_str());
		unsigned char const *end=begin+s.size();
		std::vector<unsigned char> out(encoded_size(s.size())+1);
		encode(begin,end,&out.front());
		out.back()=0;
		char const *buf=reinterpret_cast<char const *>(&out.front());
		os<<buf;
	}

	struct date::_data {};
	struct time::_data {};
	struct datetime::_data {};
	struct strftime::_data {};

	date::date() {}
	datetime::datetime() {}
	time::time()  {}
	strftime::strftime()  {}
	
	date::~date() {}
	datetime::~datetime() {}
	time::~time() {}
	strftime::~strftime() {}
	
	date::date(date const &other) : time_(other.time_),tz_(other.tz_) {}
	time::time(time const &other) : time_(other.time_),tz_(other.tz_) {}
	datetime::datetime(datetime const &other) : time_(other.time_),tz_(other.tz_) {}
	strftime::strftime(strftime const &other) : time_(other.time_),tz_(other.tz_),format_(other.format_) {}

	date const &date::operator=(date const &other) { time_=other.time_; tz_ = other.tz_; return *this; }
	time const &time::operator=(time const &other) { time_=other.time_; tz_ = other.tz_; return *this; }
	datetime const &datetime::operator=(datetime const &other) { time_=other.time_; tz_ = other.tz_; return *this; }
	strftime const &strftime::operator=(strftime const &other) { time_=other.time_; tz_ = other.tz_; format_ = other.format_; return *this; }

	date::date(streamable const &t) : time_(t) {}
	time::time(streamable const &t) : time_(t) {}
	datetime::datetime(streamable const &t) : time_(t) {}
	strftime::strftime(streamable const &t,std::string const &fmt) : time_(t),format_(fmt) {}

	date::date(streamable const &t,std::string const &tz) : time_(t),tz_(tz) {}
	time::time(streamable const &t,std::string const &tz) : time_(t),tz_(tz) {}
	datetime::datetime(streamable const &t,std::string const &tz) : time_(t),tz_(tz) {}
	strftime::strftime(streamable const &t,std::string const &fmt,std::string const &tz) : time_(t),tz_(tz),format_(fmt) {}

	void date::operator()(std::ostream &out) const
	{
		std::ostringstream ss;
		ss.copyfmt(out);
		if(!tz_.empty())
			ss << cppcms::locale::as::time_zone(tz_);
		ss << cppcms::locale::as::date;
		time_(ss);
		out << ss.str();

	}
	
	void time::operator()(std::ostream &out) const
	{
		std::ostringstream ss;
		ss.copyfmt(out);
		if(!tz_.empty())
			ss << cppcms::locale::as::time_zone(tz_);
		ss << cppcms::locale::as::time;
		time_(ss);
		out << ss.str();
	}
	
	void datetime::operator()(std::ostream &out) const
	{
		std::ostringstream ss;
		ss.copyfmt(out);
		if(!tz_.empty())
			ss << cppcms::locale::as::time_zone(tz_);
		ss << cppcms::locale::as::datetime;
		time_(ss);
		out << ss.str();
	}
	
	void strftime::operator()(std::ostream &out) const
	{
		std::ostringstream ss;
		ss.copyfmt(out);
		if(!tz_.empty())
			ss << cppcms::locale::as::time_zone(tz_);
		ss << cppcms::locale::as::datetime << cppcms::locale::as::ftime(format_);
		time_(ss);
		out << ss.str();
	}


}} // cppcms::filters
