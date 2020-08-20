//
//  Copyright (C) 2009-2020 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/regex.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <regex>

namespace booster {
	bool regex::utf8_supported()
	{
		return false;
	}
	struct regex::data {
		std::string expression;
		std::regex r;
		int flags;
		bool not_empty;
		int match_size;


		data() : 
			flags(0),
			not_empty(false),
			match_size(0) 
		{
		}

		void swap(data &other)
		{
			expression.swap(other.expression);
			r.swap(other.r);
			std::swap(flags,	other.flags);
			std::swap(not_empty,	other.not_empty);
			std::swap(match_size,	other.match_size);
		}

		data(data const &other) :
			expression(other.expression),
			r(other.r),
			flags(other.flags),
			not_empty(other.not_empty),
			match_size(other.match_size)
		{
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
		}
			
	};
	
	regex::regex() : d(new data()) 
	{
	}
	regex::regex(regex &&other) : d(std::move(other.d))
	{
	}
	regex &regex::operator=(regex &&other)
	{
		d = std::move(other.d);
		return *this;
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
		std::regex::flag_type stdflag = std::regex_constants::ECMAScript;
		if(flags & icase) 
			stdflag|= std::regex_constants::icase;
		if(flags & utf8)
			throw regex_error("std::regex does not support UTF-8, please build with PCRE with UTF-8 support");
		d->r=std::regex(pattern,stdflag);
		d->not_empty=true;
		d->match_size=d->r.mark_count();
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
		return d->match_size;
	}

	bool regex::search(char const *begin,char const *end,int /*flags*/) const
	{
		return std::regex_search(begin,end,d->r);
	}

	bool regex::search(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int /* flags */) const
	{
		if(empty())
			throw regex_error("booster::regex: Empty expression");
		std::cmatch cm;
		if(!std::regex_search(begin,end,cm,d->r))
			return false;
		int pat_size = mark_count() + 1;
		marks.clear();
		marks.resize(pat_size,std::pair<int,int>(-1,-1));
		for(int i=0;i<pat_size;i++) {
			if(cm[i].matched) {
				marks[i].first=cm[i].first - begin;
				marks[i].second = cm[i].second - begin;
			}
		}
		return true;
	}

	bool regex::empty() const
	{
		return !d->not_empty;
	}
	
	bool regex::match(char const *begin,char const *end,int /*flags*/) const
	{
		if(empty())
			throw regex_error("booster::regex: Empty expression");
		return std::regex_match(begin,end,d->r);
	}

	bool regex::match(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int /*flags*/) const
	{
		if(empty())
			throw regex_error("booster::regex: Empty expression");
		std::cmatch cm;
		if(!std::regex_match(begin,end,cm,d->r))
			return false;
		int pat_size = mark_count() + 1;
		marks.clear();
		marks.resize(pat_size,std::pair<int,int>(-1,-1));
		for(int i=0;i<pat_size;i++) {
			if(cm[i].matched) {
				marks[i].first=cm[i].first - begin;
				marks[i].second = cm[i].second - begin;
			}
		}
		return true;
	}


} // booster
