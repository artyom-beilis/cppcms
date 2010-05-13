//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/regex.h>
#include <string.h>
#include <pcre.h>
#include <sstream>

namespace booster {
	struct regex::data {
		std::string expression;
		int flags;
		pcre *re;
		int re_size;
		int match_size;


		data() : flags(0), re(0), re_size(0),match_size(0) 
		{
		}

		data(data const &other) :
			expression(other.expression),
			flags(other.flags),
			re(other.re),
			re_size(other.re_size),
			match_size(other.match_size)
		{
			if(re!=0) {
				re = (pcre *)(pcre_malloc(re_size));
				if(!re) {
					throw std::bad_alloc();
				}
				memcpy(re,other.re,re_size);
			}
		}
		data const &operator=(data const &other) 
		{
			if(this != &other) {
				if(re) pcre_free(re);
				re=0;
				re = (pcre*)pcre_malloc(other.re_size);
				if(!re) throw std::bad_alloc();
				expression = other.expression;
				flags = other.flags;
				memcpy(re,other.re,other.re_size);
				re_size = other.re_size;
				match_size = other.match_size;
			}
			return *this;
		}
		~data()
		{
			if(re) pcre_free(re);
		}
			
	};


	regex::regex() : d(new data()) 
	{
	}
	regex::regex(regex const &other) : d(other.d) 
	{
	}
	regex const &regex::operator=(regex const &other)
	{
		d=other.d;
		return *this;
	}
	regex::~regex()
	{
	}
	regex::regex(std::string const &pattern,int flags) 
	{
		assign(pattern,flags);
	}

	void regex::assign(std::string const &pattern,int flags)
	{
		d.reset(new data());
		d->expression=pattern;
		d->flags = flags;
		char const *err_ptr = 0;
		int offset = 0;
		pcre *p=pcre_compile(pattern.c_str(),0,&err_ptr,&offset,0);
		if(!p) {
			std::ostringstream ss;
			ss << err_ptr <<", at offset "<<offset;
			throw regex_error(ss.str());
		}
		d->re = p;
		if(	pcre_fullinfo(d->re,NULL,PCRE_INFO_SIZE,&d->re_size) < 0
			|| pcre_fullinfo(d->re,NULL,PCRE_INFO_CAPTURECOUNT,&d->match_size) < 0)
		{
			throw regex_error("Internal error");
		}
	}

	int regex::flags() const
	{
		return d->flags;
	}
	std::string regex::str() const
	{
		return d->expression;
	}

	unsigned regex::mark_count() const
	{
		if(d->match_size < 0)
			return 0;
		return d->match_size;
	}

	bool regex::search(char const *begin,char const *end,int /*flags*/) const
	{
		if(!d->re)
			throw regex_error("Empty expression");
		int res = pcre_exec(d->re,0,begin,end-begin,0,0,0,0);
		if(res < 0)
			return false;
		return true;
	}

	bool regex::search(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int /* flags */) const
	{
		if(!d->re)
			throw regex_error("Empty expression");
		marks.clear();
		int pat_size = mark_count() + 1;
		marks.resize(pat_size,std::pair<int,int>(-1,-1));

		std::vector<int> ovec((mark_count()+1)*3,0);
		int res = pcre_exec(d->re,0,begin,end-begin,0,0,&ovec.front(),ovec.size());
		if(res < 0)
			return false;
		for(int i=0;i<pat_size && i < res;i++) {
			marks[i].first=ovec[i*2];
			marks[i].second = ovec[i*2+1];
		}
		return true;
	}
	
	bool regex::match(char const *begin,char const *end,int /*flags*/) const
	{
		if(!d->re)
			throw regex_error("Empty expression");
		int ovec[3];
		int res = pcre_exec(d->re,0,begin,end-begin,0,PCRE_ANCHORED,ovec,3);
		if(res < 0)
			return false;
		if(ovec[0]!=0 || ovec[1]!=end-begin)
			return false;
		return true;
	}

	bool regex::match(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int /*flags*/) const
	{
		if(!d->re)
			throw regex_error("Empty expression");
		marks.clear();
		int pat_size = mark_count() + 1;
		marks.resize(pat_size,std::pair<int,int>(-1,-1));

		std::vector<int> ovec((mark_count()+1)*3,0);
		int res = pcre_exec(d->re,0,begin,end-begin,0,PCRE_ANCHORED,&ovec.front(),ovec.size());
		if(res < 0)
			return false;
		for(int i=0;i<pat_size && i < res;i++) {
			marks[i].first=ovec[i*2];
			marks[i].second = ovec[i*2+1];
		}
		return true;
	}


} // booster
