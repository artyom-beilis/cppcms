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

#ifndef CPPCMS_IMPL_REWRITE_H
#define CPPCMS_IMPL_REWRITE_H

#include <cppcms/defs.h>
#include <booster/regex.h>
#include <cppcms/json.h>
#include <cppcms/cppcms_error.h>

#include "string_map.h"

namespace cppcms {
namespace impl {

class url_rewriter {
	struct rule {
		rule(std::string const &r,std::string const &pat,bool fin=true) :
			expression(r),
			final(fin)
		{
			size_t pos = 0;
			bool append = false;
			for(;;){
				size_t start = pos;
				pos = pat.find('$',pos);
				std::string subpat = pat.substr(start,pos-start);
				if(append)
					pattern.back().append(subpat);
				else
					pattern.push_back(subpat);
				if(pos==std::string::npos) 
					break;
				pos++;
				char c;
				if(pos >= pat.size() || ((c=pat[pos++])!='$' && c < '0' && '9' < c)) 
					throw cppcms_error("Invalid rewrite pattern :" + pat);
				if(c=='$') {
					pattern.back()+='$';
					append=true;
				}
				else {
					index.push_back(c-'0');
					append=false;
				}

			}
			pattern_size = 0;
			for(size_t i=0;i<pattern.size();i++)
				pattern_size+=pattern[i].size();
		}
		booster::regex expression;
		std::vector<std::string> pattern;
		std::vector<int> index;
		size_t pattern_size;
		bool final;

		char *rewrite_once(booster::cmatch const &m,string_pool &pool) const
		{
			size_t total_size = pattern_size;
			for(size_t i=0;i<index.size();i++) {
				total_size += m[index[i]].length();
			}
			char *new_url = pool.alloc(total_size+1);
			char *ptr = new_url;
			for(size_t i=0;i<index.size();i++) {
				ptr = std::copy(pattern[i].begin(),pattern[i].end(),ptr);
				ptr = std::copy(m[index[i]].first,m[index[i]].second,ptr);
			}
			ptr = std::copy(pattern.back().begin(),pattern.back().end(),ptr);
			*ptr = 0;
			return new_url;
		}
	};
public:
	url_rewriter(json::array const &ar)
	{
		rules_.reserve(ar.size());
		for(size_t i=0;i<ar.size();i++) {
			std::string r = ar[i].get<std::string>("regex");
			std::string p = ar[i].get<std::string>("pattern");
			bool final = ar[i].get("final",true);
			rules_.push_back(rule(r,p,final));
		}
	}
	char *rewrite(char *url,string_pool &pool) const
	{
		booster::cmatch m;
		for(size_t i=0;i<rules_.size();i++) {
			rule const &r = rules_[i];
			if(booster::regex_match(url,m,r.expression)) {
				url = r.rewrite_once(m,pool);
				if(r.final)
					break;
			}
		}
		return url;
	}
private:
	std::vector<rule> rules_;	
};

} // impl
} // cppcms
#endif
