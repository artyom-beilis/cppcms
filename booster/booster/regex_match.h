//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_REGEX_MATCH_H
#define BOOSTER_REGEX_MATCH_H

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <string.h>

namespace booster {
	template<typename Iterator>
	class sub_match : public std::pair<Iterator,Iterator> {
	public:
		typedef Iterator iterator;
		typedef typename std::iterator_traits<Iterator>::value_type value_type;
		typedef typename std::iterator_traits<Iterator>::difference_type difference_type;

		typedef std::basic_string<value_type> string_type;
		typedef std::pair<Iterator,Iterator> pair_type;

		bool matched;
		difference_type length() const
		{
			if(matched)
				return std::distance(pair_type::first,pair_type::second);
			return 0;
		}

		operator string_type() const
		{
			return str();
		}

		string_type str() const
		{
			if(matched)
				return string_type(pair_type::first,pair_type::second);
			else
				return string_type();

		}
		int compare(sub_match const &other) const
		{
			return str().compare(other.str());
		}
		int compare(string_type const &other) const
		{
			return str().compare(other);
		}
		int compare(value_type const *s) const
		{
			return str().compare(s);
		}
		sub_match() : matched(false)
		{
		}
	};

	typedef sub_match<char const *> csub_match;
	typedef sub_match<std::string::const_iterator> ssub_match;


	template<typename Iterator>
	class match_results {
	public:
		typedef sub_match<Iterator> value_type;
		value_type operator[](int n) const
		{
			value_type r;
			if(n < 0 || n >= int(offsets_.size()))
				return r;
			if(offsets_[n].first == -1)
				return r;
			r.matched = true;
			r.first = begin_;
			r.second = begin_;
			std::advance(r.first,offsets_[n].first);
			std::advance(r.second,offsets_[n].second);
			return r;
		}
		size_t size() const
		{
			return offsets_.size();
		}
		
		value_type suffix()
		{
			value_type r;
			if(offsets_.empty())
				return r;
			r.first = begin_;
			r.second = end_;
			std::advance(r.first,offsets_.back().second);
			r.matched = r.first != r.second;
			return r;
		}

		value_type prefix()
		{
			value_type r;
			if(offsets_.empty() || offsets_[0].first == 0)
				return r;
			r.matched = true;
			r.first = begin_;
			r.second = begin_;
			std::advance(r.second,offsets_[0].first);
			return r;
		}

		void assign(Iterator begin,Iterator end,std::vector<std::pair<int,int> > &offsets)
		{
			begin_ = begin;
			end_ = end;
			offsets_.swap(offsets);
		}
	private:
		Iterator begin_,end_;
		std::vector<std::pair<int,int> > offsets_;
	};

	typedef match_results<char const *> cmatch;
	typedef match_results<std::string::const_iterator> smatch;

	template<typename Regex>
	bool regex_match(char const *begin,char const *end,cmatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		bool res = r.match(begin,end,map,flags);
		if(!res) return false;
		m.assign(begin,end,map);
		return true;
	}

	template<typename Regex>
	bool regex_match(std::string const &s,smatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		bool res = r.match(s.c_str(),s.c_str()+s.size(),map,flags);
		if(!res) return false;
		m.assign(s.begin(),s.end(),map);
		return true;
	}
	
	template<typename Regex>
	bool regex_match(char const *s,cmatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		char const *begin=s;
		char const *end = begin+strlen(begin);
		bool res = r.match(begin,end,map,flags);
		if(!res) return false;
		m.assign(begin,end,map);
		return true;
	}

	template<typename Regex>
	bool regex_search(char const *begin,char const *end,cmatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		bool res = r.search(begin,end,map,flags);
		if(!res) return false;
		m.assign(begin,end,map);
		return true;
	}

	template<typename Regex>
	bool regex_search(std::string const &s,smatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		bool res = r.search(s.c_str(),s.c_str()+s.size(),map,flags);
		if(!res) return false;
		m.assign(s.begin(),s.end(),map);
		return true;
	}
	
	template<typename Regex>
	bool regex_search(char const *s,cmatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		char const *begin=s;
		char const *end = begin+strlen(begin);
		bool res = r.search(begin,end,map,flags);
		if(!res) return false;
		m.assign(begin,end,map);
		return true;
	}

	template<typename Regex>
	bool regex_match(char const *begin,char const *end, Regex const &r,int flags = 0)
	{
		return r.match(begin,end,flags);
	}

	template<typename Regex>
	bool regex_match(std::string const &s, Regex const &r,int flags = 0)
	{
		return r.match(s.c_str(),s.c_str()+s.size(),flags);
	}
	
	template<typename Regex>
	bool regex_match(char const *s, Regex const &r,int flags = 0)
	{
		return r.match(s,s+strlen(s),flags);
	}
	
	template<typename Regex>
	bool regex_search(char const *begin,char const *end, Regex const &r,int flags = 0)
	{
		return r.search(begin,end,flags);
	}

	template<typename Regex>
	bool regex_search(std::string const &s, Regex const &r,int flags = 0)
	{
		return r.search(s.c_str(),s.c_str()+s.size(),flags);
	}
	
	template<typename Regex>
	bool regex_search(char const *s, Regex const &r,int flags = 0)
	{
		return r.search(s,s+strlen(s),flags);
	}

	
	// sub -- sub	
	template<typename Iterator>
	bool operator==(sub_match<Iterator> const &l,sub_match<Iterator> const &r) { return l.compare(r) == 0; }
	template<typename Iterator>
	bool operator!=(sub_match<Iterator> const &l,sub_match<Iterator> const &r) { return l.compare(r) != 0; }
	template<typename Iterator>
	bool operator< (sub_match<Iterator> const &l,sub_match<Iterator> const &r) { return l.compare(r) <  0; }
	template<typename Iterator>
	bool operator> (sub_match<Iterator> const &l,sub_match<Iterator> const &r) { return l.compare(r) >  0; }
	template<typename Iterator>
	bool operator<=(sub_match<Iterator> const &l,sub_match<Iterator> const &r) { return l.compare(r) <= 0; }
	template<typename Iterator>
	bool operator>=(sub_match<Iterator> const &l,sub_match<Iterator> const &r) { return l.compare(r) >= 0; }

	// str -- sub
	template<typename Iterator>
	bool operator==(
		typename sub_match<Iterator>::string_type const &l,
		sub_match<Iterator> const &r) 
	{ return l.compare(r) == 0; }
	template<typename Iterator>
	bool operator!=(
		typename sub_match<Iterator>::string_type const &l,
		sub_match<Iterator> const &r) 
	{ return l.compare(r) != 0; }
	
	template<typename Iterator>
	bool operator<=(
		typename sub_match<Iterator>::string_type const &l,
		sub_match<Iterator> const &r) 
	{ return l.compare(r) <= 0; }
	template<typename Iterator>
	bool operator>=(
		typename sub_match<Iterator>::string_type const &l,
		sub_match<Iterator> const &r) 
	{ return l.compare(r) >= 0; }
	
	template<typename Iterator>
	bool operator<(
		typename sub_match<Iterator>::string_type const &l,
		sub_match<Iterator> const &r) 
	{ return l.compare(r) <0; }
	template<typename Iterator>
	bool operator>(
		typename sub_match<Iterator>::string_type const &l,
		sub_match<Iterator> const &r) 
	{ return l.compare(r) > 0; }

	// sub -- str

	template<typename Iterator>
	bool operator==(
		sub_match<Iterator> const &l,
		typename sub_match<Iterator>::string_type const &r
		) 
	{ return l.compare(r) == 0; }
	template<typename Iterator>
	bool operator!=(
		sub_match<Iterator> const &l,
		typename sub_match<Iterator>::string_type const &r
		) 
	{ return l.compare(r) != 0; }
	
	template<typename Iterator>
	bool operator<=(
		sub_match<Iterator> const &l,
		typename sub_match<Iterator>::string_type const &r
		) 
	{ return l.compare(r) <= 0; }

	template<typename Iterator>
	bool operator>=(
		sub_match<Iterator> const &l,
		typename sub_match<Iterator>::string_type const &r
		) 
	{ return l.compare(r) >= 0; }
	
	template<typename Iterator>
	bool operator<(
		sub_match<Iterator> const &l,
		typename sub_match<Iterator>::string_type const &r
		) 
	{ return l.compare(r) <0; }

	template<typename Iterator>
	bool operator>(
		sub_match<Iterator> const &l,
		typename sub_match<Iterator>::string_type const &r
		) 
	{ return l.compare(r) > 0; }

	// * -- sub
	template<typename Iterator>
	bool operator==(
		typename std::iterator_traits<Iterator>::value_type const *l,
		sub_match<Iterator> const &r) 
	{ return r.compare(l) ==0; }
	template<typename Iterator>
	bool operator!=(
		typename std::iterator_traits<Iterator>::value_type const *l,
		sub_match<Iterator> const &r) 
	{ return r.compare(l) !=0; }


	template<typename Iterator>
	bool operator<=(
		typename std::iterator_traits<Iterator>::value_type const *l,
		sub_match<Iterator> const &r) 
	{ return r.compare(l) > 0; }
	template<typename Iterator>
	bool operator>=(
		typename std::iterator_traits<Iterator>::value_type const *l,
		sub_match<Iterator> const &r) 
	{ return r.compare(l) < 0; }
	
	template<typename Iterator>
	bool operator<(
		typename std::iterator_traits<Iterator>::value_type const *l,
		sub_match<Iterator> const &r) 
	{ return r.compare(l) >=0; }
	template<typename Iterator>
	bool operator>(
		typename std::iterator_traits<Iterator>::value_type const *l,
		sub_match<Iterator> const &r) 
	{ return r.compare(l) <= 0; }


	// sub -- *

	template<typename Iterator>
	bool operator==(
		sub_match<Iterator> const &l,
		typename std::iterator_traits<Iterator>::value_type const *r
		) 
	{ return l.compare(r) == 0; }
	template<typename Iterator>
	bool operator!=(
		sub_match<Iterator> const &l,
		typename std::iterator_traits<Iterator>::value_type const *r
		) 
	{ return l.compare(r) != 0; }
	
	template<typename Iterator>
	bool operator<=(
		sub_match<Iterator> const &l,
		typename std::iterator_traits<Iterator>::value_type const *r
		) 
	{ return l.compare(r) <= 0; }

	template<typename Iterator>
	bool operator>=(
		sub_match<Iterator> const &l,
		typename std::iterator_traits<Iterator>::value_type const *r
		) 
	{ return l.compare(r) >= 0; }
	
	template<typename Iterator>
	bool operator<(
		sub_match<Iterator> const &l,
		typename std::iterator_traits<Iterator>::value_type const *r
		) 
	{ return l.compare(r) <0; }

	template<typename Iterator>
	bool operator>(
		sub_match<Iterator> const &l,
		typename std::iterator_traits<Iterator>::value_type const *r
		) 
	{ return l.compare(r) > 0; }

	// add + 

	template<typename Iterator>
	typename sub_match<Iterator>::string_type 
	operator+(sub_match<Iterator> const &l,sub_match<Iterator> const &r)
	{ return l.str() + r.str(); }
	
	template<typename Iterator>
	typename sub_match<Iterator>::string_type 
	operator+(sub_match<Iterator> const &l,typename sub_match<Iterator>::string_type const &r)
	{ return l.str() + r; }
	
	template<typename Iterator>
	typename sub_match<Iterator>::string_type 
	operator+(typename sub_match<Iterator>::string_type const &l,sub_match<Iterator> const &r)
	{ return l + r.str(); }
	
	template<typename Iterator>
	typename sub_match<Iterator>::string_type 
	operator+(sub_match<Iterator> const &l,typename sub_match<Iterator>::value_type const *r)
	{ return l.str() + r; }
	
	template<typename Iterator>
	typename sub_match<Iterator>::string_type 
	operator+(typename sub_match<Iterator>::value_type const *l,sub_match<Iterator> const &r)
	{ return l + r.str(); }

}



#endif
