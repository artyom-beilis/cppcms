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
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <sstream>
#include <algorithm>

namespace booster {

	bool regex::utf8_supported()
	{
		uint32_t flag = 0;
		pcre2_config(PCRE2_CONFIG_UNICODE, &flag);
		return flag != 0;
	}
	struct regex::data {
		std::string expression;
		int flags;
		pcre2_code *re;
		pcre2_code *are;
		size_t re_size;
		size_t are_size;
		uint32_t match_size;


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
			if(other.re!=0) {
				re = pcre2_code_copy(other.re);
				if(!re) {
					throw std::bad_alloc();
				}
			}
			if(other.are!=0) {
				are = pcre2_code_copy(other.are);
				if(!are) {
					pcre2_code_free(re);
					throw std::bad_alloc();
				}
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
			if(re) pcre2_code_free(re);
			if(are) pcre2_code_free(are);
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
	regex::regex(regex &&other) : d(std::move(other.d))
	{
	}
	regex &regex::operator=(regex &&other)
	{
		d = std::move(other.d);
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
		int err_code = 0;
		size_t err_offset = 0;
		uint32_t pcre_flags = 0;
		if(flags & icase) {
			pcre_flags |= PCRE2_CASELESS;
		}
		if(flags & utf8) {
			pcre_flags |= (PCRE2_UTF | PCRE2_UCP);
		}
		pcre2_code *p = pcre2_compile(
			reinterpret_cast<PCRE2_SPTR>(pattern.c_str()),
			pattern.size(),
			pcre_flags,
			&err_code,
			&err_offset,
			NULL
		);
		if(!p) {
			PCRE2_UCHAR buffer[256];
			pcre2_get_error_message(err_code, buffer, sizeof(buffer));
			std::ostringstream ss;
			ss << "booster::regex:" << buffer << ", at offset " << err_offset;
			throw regex_error(ss.str());
		}
		d->re = p;
		
		if(pcre2_pattern_info(d->re, PCRE2_INFO_SIZE, &d->re_size) < 0
			|| pcre2_pattern_info(d->re, PCRE2_INFO_CAPTURECOUNT, &d->match_size) < 0)
		{
			throw regex_error("booster::regex: Internal error");
		}
		
		std::string anchored;
		anchored.reserve(pattern.size()+6);
		anchored+= "(?:";
		anchored+=pattern;
		anchored+=")\\z";

		p = pcre2_compile(
			reinterpret_cast<PCRE2_SPTR>(anchored.c_str()),
			anchored.size(),
			pcre_flags,
			&err_code,
			&err_offset,
			NULL
		);
		if(!p) {
			throw regex_error("booster::regex: Internal error");
		}
		d->are = p;
		if(pcre2_pattern_info(d->are, PCRE2_INFO_SIZE, &d->are_size) != 0)
		{
			throw regex_error("booster::regex: Internal error");
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
		return d->match_size;
	}

	bool regex::search(char const *begin,char const *end,int /*flags*/) const
	{
		if(!d->re)
			throw regex_error("booster::regex: Empty expression");
		
		pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(d->re, NULL);
		if (!match_data) return false;

		int res = pcre2_match(
			d->re,
			reinterpret_cast<PCRE2_SPTR>(begin),
			end - begin,
			0,
			0,
			match_data,
			NULL
		);
		
		pcre2_match_data_free(match_data);
		return res >= 0;
	}

	bool regex::search(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int /* flags */) const
	{
		if(!d->re)
			throw regex_error("booster::regex: Empty expression");
		marks.clear();
		int pat_size = mark_count() + 1;
		marks.resize(pat_size,std::pair<int,int>(-1,-1));

		pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(d->re, NULL);
		if (!match_data) return false;

		int res = pcre2_match(
			d->re,
			reinterpret_cast<PCRE2_SPTR>(begin),
			end - begin,
			0,
			0,
			match_data,
			NULL
		);
		
		if(res < 0) {
			pcre2_match_data_free(match_data);
			return false;
		}

		size_t *ovector = pcre2_get_ovector_pointer(match_data);
		for(int i=0; i<pat_size && i < res; i++) {
			marks[i].first = static_cast<int>(ovector[i*2]);
			marks[i].second = static_cast<int>(ovector[i*2+1]);
		}
		
		pcre2_match_data_free(match_data);
		return true;
	}

	bool regex::empty() const
	{
		return d->re == 0;
	}
	
	bool regex::match(char const *begin,char const *end,int /*flags*/) const
	{
		if(!d->are)
			throw regex_error("booster::regex: Empty expression");
		
		pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(d->are, NULL);
		if (!match_data) return false;

		int res = pcre2_match(
			d->are,
			reinterpret_cast<PCRE2_SPTR>(begin),
			end - begin,
			0,
			PCRE2_ANCHORED,
			match_data,
			NULL
		);
		
		pcre2_match_data_free(match_data);
		return res >= 0;
	}

	bool regex::match(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int /*flags*/) const
	{
		if(!d->are)
			throw regex_error("booster::regex: Empty expression");
		marks.clear();
		int pat_size = mark_count() + 1;
		marks.resize(pat_size,std::pair<int,int>(-1,-1));

		pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(d->are, NULL);
		if (!match_data) return false;

		int res = pcre2_match(
			d->are,
			reinterpret_cast<PCRE2_SPTR>(begin),
			end - begin,
			0,
			PCRE2_ANCHORED,
			match_data,
			NULL
		);
		
		if(res < 0) {
			pcre2_match_data_free(match_data);
			return false;
		}
		
		size_t *ovector = pcre2_get_ovector_pointer(match_data);
		if(ovector[0]!=0 || ovector[1]!=static_cast<size_t>(end-begin)) {
			pcre2_match_data_free(match_data);
			return false;
		}
			
		for(int i=0; i<pat_size && i < res; i++) {
			marks[i].first = static_cast<int>(ovector[i*2]);
			marks[i].second = static_cast<int>(ovector[i*2+1]);
		}
		
		pcre2_match_data_free(match_data);
		return true;
	}

} // booster
