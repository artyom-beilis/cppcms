#define CPPCMS_SOURCE
#include "filters.h"
#include "base64.h"
#include <iostream>

namespace cppcms { namespace filters {
	
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



	date::date(std::tm const &t) : tm_(t) {}
	time::time(std::tm const &t) : tm_(t) {}
	datetime::datetime(std::tm const &t) : tm_(t) {}

	void date::operator()(std::ostream &out) const
	{
		if(out.getloc().name()=="*")
			out<<strftime("%Y-%m-%d",tm_);
		else
			out<<strftime("%x",tm_);
	}
	
	void time::operator()(std::ostream &out) const
	{
		if(out.getloc().name()=="*")
			out<<strftime("%H:%M:%S",tm_);
		else
			out<<strftime("%X",tm_);
	}
	
	void datetime::operator()(std::ostream &out) const
	{
		if(out.getloc().name()=="*")
			out<<strftime("%Y-%m-%d %H:%M:%S",tm_);
		else
			out<<strftime("%c",tm_);
	}

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


}} // cppcms::filters
