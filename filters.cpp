#define CPPCMS_SOURCE
#include "filters.h"
#include "base64.h"
#include "locale_convert.h"
#include "util.h"
#include <iostream>

namespace cppcms { namespace filters {

	struct streamable::data {};
	streamable::streamable() 
	{
		set(0,0,0);
	}
	streamable::streamable(streamable const &other)
	{
		set(other.ptr_,other.to_stream_,other.to_string_);
	}
	streamable::~streamable()
	{
	}
	streamable const &streamable::operator=(streamable const &other)
	{
		if(&other!=this)
			set(other.ptr_,other.to_stream_,other.to_string_);
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
		set(ptr,ch_to_stream,ch_to_string);
	}
	template<>
	streamable::streamable(std::string const &str)
	{
		set(&str,to_stream<std::string>,s_to_string);
	}
	std::string streamable::get(std::ios &ios) const
	{
		return to_string_(ios,ptr_);
	}
	void streamable::operator()(std::ostream &out) const
	{
		to_stream_(out,ptr_);
	}
	void streamable::set(void const *ptr,to_stream_type tse,to_string_type tst)
	{
		ptr_=ptr;
		to_stream_=tse;
		to_string_=tst;
	}

///////////////////////////////////
	
	struct to_upper::data {};
	to_upper::to_upper() {}
	to_upper::~to_upper() {}
	to_upper::to_upper(to_upper const &other) : obj_(other.obj_) {}
	to_upper::to_upper(streamable const &obj) : obj_(obj) {}
	to_upper const &to_upper::operator=(to_upper const &other){ obj_ = other.obj_; return *this; }
	void to_upper::operator()(std::ostream &out) const
	{
		out << std::use_facet<locale::convert>(out.getloc()).to_upper(obj_.get(out));
	}

	struct to_lower::data {};
	to_lower::to_lower() {}
	to_lower::~to_lower() {}
	to_lower::to_lower(to_lower const &other) : obj_(other.obj_) {}
	to_lower::to_lower(streamable const &obj) : obj_(obj) {}
	to_lower const &to_lower::operator=(to_lower const &other){ obj_ = other.obj_; return *this; }
	void to_lower::operator()(std::ostream &out) const
	{
		out << std::use_facet<locale::convert>(out.getloc()).to_lower(obj_.get(out));
	}

	struct to_title::data {};
	to_title::to_title() {}
	to_title::~to_title() {}
	to_title::to_title(to_title const &other) : obj_(other.obj_) {}
	to_title::to_title(streamable const &obj) : obj_(obj) {}
	to_title const &to_title::operator=(to_title const &other){ obj_ = other.obj_; return *this; }
	void to_title::operator()(std::ostream &out) const
	{
		out << std::use_facet<locale::convert>(out.getloc()).to_title(obj_.get(out));
	}

	struct escape::data {};
	escape::escape() {}
	escape::~escape() {}
	escape::escape(escape const &other) : obj_(other.obj_) {}
	escape::escape(streamable const &obj) : obj_(obj) {}
	escape const &escape::operator=(escape const &other){ obj_ = other.obj_; return *this; }
	void escape::operator()(std::ostream &out) const
	{
		out << util::escape(obj_.get(out));
	}

	struct urlencode::data {};
	urlencode::urlencode() {}
	urlencode::~urlencode() {}
	urlencode::urlencode(urlencode const &other) : obj_(other.obj_) {}
	urlencode::urlencode(streamable const &obj) : obj_(obj) {}
	urlencode const &urlencode::operator=(urlencode const &other){ obj_ = other.obj_; return *this; }
	void urlencode::operator()(std::ostream &out) const
	{
		out << util::urlencode(obj_.get(out));
	}

	struct raw::data {};
	raw::raw() {}
	raw::~raw() {}
	raw::raw(raw const &other) : obj_(other.obj_) {}
	raw::raw(streamable const &obj) : obj_(obj) {}
	raw const &raw::operator=(raw const &other){ obj_ = other.obj_; return *this; }
	void raw::operator()(std::ostream &out) const
	{
		out << obj_;;
	}

	struct base64_urlencode::data {};
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
		char const *buf=reinterpret_cast<char const *>(out.front());
		os<<buf;
	}

	struct strftime::data {};
	strftime::strftime() {}
	strftime::~strftime() {}
	strftime::strftime(strftime const &other) : format_(other.format_),t_(other.t_) {}
	strftime::strftime(streamable const &obj,std::tm const &t) : format_(obj),t_(&t) {}
	strftime const &strftime::operator=(strftime const &other)
	{
		format_ = other.format_;
		t_=other.t_;
		return *this;
	}
	void strftime::operator()(std::ostream &out) const
	{
		std::string fmt=format_.get(out);
		std::use_facet<std::time_put<char> >(out.getloc()).put(out,out,' ',t_,fmt.data(),fmt.data()+fmt.size());
	}


	void format::init(streamable const &f)
	{
		format_=f;
		size_ = 0;
	}
	streamable const *format::at(size_t n) const
	{
		n--;
		if(n >= size_ || n < 0)
			return 0;

		const size_t objects_size =  sizeof(objects_) / sizeof(objects_[0]);

		if(n < objects_size)
			return &objects_[n];
		return &vobjects_[n - objects_size];
	}
	format &format::add(streamable const &obj)
	{
		if(size_ >= sizeof(objects_) / sizeof(objects_[0]))
			vobjects_.push_back(obj);
		else
			objects_[size_] = obj;
		size_ ++;
		return *this;
	}
	void format::write(std::ostream &output) const
	{
		int pos = 0;
		std::string const fmt=format_.get(output);
		for(std::string::const_iterator p=fmt.begin(),e=fmt.end();p!=e;) {
			char c=*p++;
			if(c=='%') {
				format_one(output,p,e,pos);
			}
			else
				output.put(c);
		}
	}

	void format::format_one(std::ostream &out,std::string::const_iterator &p,std::string::const_iterator e,int &pos) const
	{
		if(p==e)
			return;
		if(*p == '%') {
			++p;
			out << '%';
			return;
		}
		pos++;
		int the_pos = pos;

		if('1' <= *p  && *p<='9') {
			int n=0;
			while(p!=e && '0' <= *p  && *p<='9') {
				n=n*10 + (*p - '0');
				++p;
			}
			if(p==e)
				return;
			if(*p=='%') {
				++p;
				the_pos = n;
				streamable const *obj=at(the_pos);
				if(obj) {
					out<<*obj;
				}
				return;
			}
			return;
		}
	}

	std::string format::str(std::locale const &loc) const
	{
		std::ostringstream oss;
		oss.imbue(loc);
		write(oss);
		return oss.str();
	}

	struct date::data {};
	struct time::data {};
	struct datetime::data {};

	date::date() : t_(0) {}
	datetime::datetime() : t_(0){}
	time::time() : t_(0) {}
	
	date::~date() {}
	datetime::~datetime() {}
	time::~time() {}
	
	date::date(date const &other) : t_(other.t_) {}
	time::time(time const &other) : t_(other.t_) {}
	datetime::datetime(datetime const &other) : t_(other.t_) {}

	date const &date::operator=(date const &other) { t_=other.t_; return *this; }
	time const &time::operator=(time const &other) { t_=other.t_; return *this; }
	datetime const &datetime::operator=(datetime const &other) { t_=other.t_; return *this; }

	date::date(std::tm const &t) : t_(&t) {}
	time::time(std::tm const &t) : t_(&t) {}
	datetime::datetime(std::tm const &t) : t_(&t) {}

	void date::operator()(std::ostream &out) const
	{
		if(out.getloc().name()=="*")
			out<<strftime("%Y-%m-%d",*t_);
		else
			out<<strftime("%x",*t_);
	}
	
	void time::operator()(std::ostream &out) const
	{
		if(out.getloc().name()=="*")
			out<<strftime("%H:%M:%S",*t_);
		else
			out<<strftime("%X",*t_);
	}
	
	void datetime::operator()(std::ostream &out) const
	{
		if(out.getloc().name()=="*")
			out<<strftime("%Y-%m-%d %H:%M:%S",*t_);
		else
			out<<strftime("%c",*t_);
	}


}} // cppcms::filters
