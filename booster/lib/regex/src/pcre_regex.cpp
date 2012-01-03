//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/regex.h>
#include <string.h>
#include <pcre.h>
#include <sstream>
#include <algorithm>

namespace booster {
	struct regex::data {
		std::string expression;
		int flags;
		pcre *re;
		pcre *are;
		size_t re_size;
		size_t are_size;
		int match_size;


		data() : 
			flags(0),
			re(0),
			are(0),
			re_size(0),
			are_size(0),
			match_size(0) 
		{
		}

		void swap(data &other)
		{
			expression.swap(other.expression);
			std::swap(flags,	other.flags);
			std::swap(re,		other.re);
			std::swap(are,		other.are);
			std::swap(re_size,	other.re_size);
			std::swap(are_size,	other.are_size);
			std::swap(match_size,	other.match_size);
		}

		data(data const &other) :
			expression(other.expression),
			flags(other.flags),
			re(0),
			are(0),
			re_size(other.re_size),
			are_size(other.are_size),
			match_size(other.match_size)
		{
			try {
				if(other.re!=0) {
					re = (pcre *)(pcre_malloc(re_size));
					if(!re) {
						throw std::bad_alloc();
					}
					memcpy(re,other.re,re_size);
				}
				if(other.are!=0) {
					are = (pcre *)(pcre_malloc(are_size));
					if(!are) {
						throw std::bad_alloc();
					}
					memcpy(are,other.are,are_size);
				}
			}
			catch(...) {
				if(re) pcre_free(re);
				if(are) pcre_free(are);
				throw;
			}
		}
		data const &operator=(data const &other) 
		{
			if(this != &other) {
				data tmp(other);
				swap(tmp);
			}
			return *this;
		}
		~data()
		{
			if(re) pcre_free(re);
			if(are) pcre_free(are);
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
		
		std::string anchored;
		anchored.reserve(pattern.size()+6);
		anchored+= "(?:";
		anchored+=pattern;
		anchored+=")\\z";

		p=pcre_compile(anchored.c_str(),0,&err_ptr,&offset,0);
		if(!p) {
			throw regex_error("Internal error");
		}
		d->are = p;
		if(pcre_fullinfo(d->are,NULL,PCRE_INFO_SIZE,&d->are_size) != 0)
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

	bool regex::empty() const
	{
		return d->re == 0;
	}
	
	bool regex::match(char const *begin,char const *end,int /*flags*/) const
	{
		if(!d->are)
			throw regex_error("Empty expression");
		
		int res = pcre_exec(d->are,0,begin,end-begin,0,PCRE_ANCHORED,0,0);
		if(res < 0)
			return false;
		return true;
	}

	bool regex::match(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int /*flags*/) const
	{
		if(!d->are)
			throw regex_error("Empty expression");
		marks.clear();
		int pat_size = mark_count() + 1;
		marks.resize(pat_size,std::pair<int,int>(-1,-1));

		std::vector<int> ovec((mark_count()+1)*3,0);
		int res = pcre_exec(d->are,0,begin,end-begin,0,PCRE_ANCHORED,&ovec.front(),ovec.size());
		if(res < 0)
			return false;
		if(ovec[0]!=0 || ovec[1]!=end-begin)
			return false;
		for(int i=0;i<pat_size && i < res;i++) {
			marks[i].first=ovec[i*2];
			marks[i].second = ovec[i*2+1];
		}
		return true;
	}


} // booster
