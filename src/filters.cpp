///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/filters.h>
#include <cppcms/steal_buf.h>
#include <cppcms/base64.h>
#include <cppcms/util.h>
#include <iostream>

namespace cppcms { 

struct translation_domain_scope::_data {};

void translation_domain_scope::set_and_save(int new_id)
{
	if(new_id < 0)
		return;
	booster::locale::ios_info &info = booster::locale::ios_info::get(*output_);
	prev_id_ = info.domain_id();
	info.domain_id(new_id);
}

int translation_domain_scope::domain_id(std::ostream &out,std::string const &dom)
{
	return std::use_facet<booster::locale::message_format<char> >(out.getloc()).domain(dom);
}

translation_domain_scope::translation_domain_scope(std::ostream &out,std::string const &dom) : 
	output_(&out),
	prev_id_(-1)
{
	set_and_save(domain_id(out,dom));
}

translation_domain_scope::translation_domain_scope(std::ostream &out,int new_id) : 
	output_(&out),
	prev_id_(-1)
{
	set_and_save(new_id);
}

translation_domain_scope::~translation_domain_scope()
{
	if(prev_id_==-1)
		return;
	try {
		booster::locale::ios_info::get(*output_).domain_id(prev_id_);
	}
	catch(...) {}
}

	
namespace filters {

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
		std::string ch_to_string(std::ios &/*ios*/,void const *p)
		{
			return reinterpret_cast<char const *>(p);
		}
		std::string s_to_string(std::ios &/*ios*/,void const *p)
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
		util::steal_buffer<> sb(out);
		obj_(out);
		sb.release();
		out << ::cppcms::locale::to_upper( sb.begin(),sb.end(),out.getloc());
	}

	struct to_lower::_data {};
	to_lower::to_lower() {}
	to_lower::~to_lower() {}
	to_lower::to_lower(to_lower const &other) : obj_(other.obj_) {}
	to_lower::to_lower(streamable const &obj) : obj_(obj) {}
	to_lower const &to_lower::operator=(to_lower const &other){ obj_ = other.obj_; return *this; }
	void to_lower::operator()(std::ostream &out) const
	{
		util::steal_buffer<> sb(out);
		obj_(out);
		sb.release();
		out << ::cppcms::locale::to_lower( sb.begin(),sb.end(),out.getloc());
	}

	struct to_title::_data {};
	to_title::to_title() {}
	to_title::~to_title() {}
	to_title::to_title(to_title const &other) : obj_(other.obj_) {}
	to_title::to_title(streamable const &obj) : obj_(obj) {}
	to_title const &to_title::operator=(to_title const &other){ obj_ = other.obj_; return *this; }
	void to_title::operator()(std::ostream &out) const
	{
		util::steal_buffer<> sb(out);
		obj_(out);
		sb.release();
		out << ::cppcms::locale::to_title( sb.begin(),sb.end(),out.getloc());
	}

	namespace {
		struct escape_buf : public util::filterbuf<escape_buf,128> {
		public:
			int convert(char const *begin,char const *end,std::streambuf *out)
			{
				if(!out)
					return -1;
				return util::escape(begin,end,*out);
			}
		};
	}

	struct escape::_data {};
	escape::escape() {}
	escape::~escape() {}
	escape::escape(escape const &other) : obj_(other.obj_) {}
	escape::escape(streamable const &obj) : obj_(obj) {}
	escape const &escape::operator=(escape const &other){ obj_ = other.obj_; return *this; }
	void escape::operator()(std::ostream &out) const
	{
		escape_buf eb;
		eb.steal(out);
		obj_(out);
		eb.release();
	}

	namespace {
		struct jsescape_buf : public util::filterbuf<jsescape_buf,128> {
		public:
			jsescape_buf() 
			{
				buf_[0]='\\';
				buf_[1]= 'u';
				buf_[2]= '0';
				buf_[3]= '0';
				// 4 first digit
				// 5 2nd digit
				buf_[6]='\0';
			}
			char const *encode(unsigned char c)
			{
				static char const tohex[]="0123456789abcdef";
				buf_[4]=tohex[c >>  4];
				buf_[5]=tohex[c & 0xF];
				return buf_;
			}
			int convert(char const *begin,char const *end,std::streambuf *out)
			{
				if(!out)
					return -1;
				while(begin!=end) {
					char const *addon = 0;
					unsigned char c=*begin++;
					switch(c) {
					case 0x22: addon = "\\\""; break;
					case 0x5C: addon = "\\\\"; break;
					case '\b': addon = "\\b"; break;
					case '\f': addon = "\\f"; break;
					case '\n': addon = "\\n"; break;
					case '\r': addon = "\\r"; break;
					case '\t': addon = "\\t"; break;
					case '\'': addon = encode(c); break;
					default:
						if(c<=0x1F) 
							addon=encode(c);
						else {
							if(out->sputc(c)==EOF)
								return -1;
							continue;
						}
					}
					while(*addon)
						if(out->sputc(*addon++)==EOF)
							return -1;

				}
				return 0;
			}
		private:
			char buf_[8];
		};
	}

	struct jsescape::_data {};
	jsescape::jsescape() {}
	jsescape::~jsescape() {}
	jsescape::jsescape(jsescape const &other) : obj_(other.obj_) {}
	jsescape::jsescape(streamable const &obj) : obj_(obj) {}
	jsescape const &jsescape::operator=(jsescape const &other){ obj_ = other.obj_; return *this; }
	void jsescape::operator()(std::ostream &out) const
	{
		jsescape_buf sb;
		sb.steal(out);
		obj_(out);
		sb.release();
	}
	
	
	namespace {
		struct urlencode_buf : public util::filterbuf<urlencode_buf,128> {
		public:
			int convert(char const *begin,char const *end,std::streambuf *out)
			{
				if(!out)
					return -1;
				return util::urlencode(begin,end,*out);
			}
		};
	}





	struct urlencode::_data {};
	urlencode::urlencode() {}
	urlencode::~urlencode() {}
	urlencode::urlencode(urlencode const &other) : obj_(other.obj_) {}
	urlencode::urlencode(streamable const &obj) : obj_(obj) {}
	urlencode const &urlencode::operator=(urlencode const &other){ obj_ = other.obj_; return *this; }
	void urlencode::operator()(std::ostream &out) const
	{
		urlencode_buf sb;
		sb.steal(out);
		obj_(out);
		sb.release();
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
	void base64_urlencode::operator()(std::ostream &out) const
	{
		util::steal_buffer<> sb(out);
		obj_(out);
		sb.release();
		using namespace cppcms::b64url;
		unsigned char const *begin=reinterpret_cast<unsigned char const *>(sb.begin());
		unsigned char const *end=  reinterpret_cast<unsigned char const *>(sb.end());
		b64url::encode(begin,end,out);
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
