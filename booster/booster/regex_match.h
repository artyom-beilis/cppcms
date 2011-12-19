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

	///
	/// \brief This class represents a single captures subexpression.
	///
	/// The subexpressions captured text is found between [first,second). 
	///
	template<typename Iterator>
	class sub_match : public std::pair<Iterator,Iterator> {
	public:
		typedef Iterator iterator;
		typedef typename std::iterator_traits<Iterator>::value_type value_type;
		typedef typename std::iterator_traits<Iterator>::difference_type difference_type;

		///
		/// The string type that this expression can be converted into, generally std::string.
		///
		typedef std::basic_string<value_type> string_type;
		typedef std::pair<Iterator,Iterator> pair_type;

		///
		/// This flag is true if the expression was matched, false otherwise.
		/// if matched is false then there is no guarantees that first or second are valid iterators.
		///
		bool matched;

		///
		/// The length of captured subexpression, 0 if matched==false.
		///
		difference_type length() const
		{
			if(matched)
				return std::distance(pair_type::first,pair_type::second);
			return 0;
		}

		///
		/// Explicit conversion operator to string
		///
		operator string_type() const
		{
			return str();
		}

		///
		/// Convert the subexpression to string. If matched is false, return empty string.
		///
		string_type str() const
		{
			if(matched)
				return string_type(pair_type::first,pair_type::second);
			else
				return string_type();

		}
		///
		/// Compare two subexpressions. Same as str().compare(other.str())
		///
		int compare(sub_match const &other) const
		{
			return str().compare(other.str());
		}
		///
		/// Compare two subexpressions. Same as str().compare(other)
		///
		int compare(string_type const &other) const
		{
			return str().compare(other);
		}
		///
		/// Compare two subexpressions. Same as str().compare(s)
		///
		int compare(value_type const *s) const
		{
			return str().compare(s);
		}
		///
		/// Default not-matched subexpressions.
		///
		sub_match() : matched(false)
		{
		}
	};

	typedef sub_match<char const *> csub_match;
	typedef sub_match<std::string::const_iterator> ssub_match;


	///
	/// \brief The object that hold the result of matching a regular expression against the text
	/// using regex_match and regex_search functions
	///
	template<typename Iterator>
	class match_results {
	public:
		///
		/// Creates default empty matched result.
		///
		match_results()
		{
			begin_ = Iterator();
			end_ = Iterator();
		}
		///
		/// The type of subexpression returned by operator[]
		///
		typedef sub_match<Iterator> value_type;

		///
		/// Get the sub_match for subexpression \a n. If n < 0 or n >= size() returns an empty sub_match
		///
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

		///
		/// Get the number of captured subexpressions in the regular expression.
		///
		size_t size() const
		{
			return offsets_.size();
		}
		
		///
		/// Get the text range before the matched expression. Always empty for match_results
		///	
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

		///
		/// Get the text range after the matched expression. Always empty for match_results
		///	
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

		/// \cond INTERNAL

		void assign(Iterator begin,Iterator end,std::vector<std::pair<int,int> > &offsets)
		{
			begin_ = begin;
			end_ = end;
			offsets_.swap(offsets);
		}

		/// \endcond

	private:
		Iterator begin_,end_;
		std::vector<std::pair<int,int> > offsets_;
	};

	typedef match_results<char const *> cmatch;
	typedef match_results<std::string::const_iterator> smatch;

	///
	/// Match an expression \a r against text in range [\a begin, \a end ), return true
	/// if found and store matched patters in \a m
	///
	template<typename Regex>
	bool regex_match(char const *begin,char const *end,cmatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		bool res = r.match(begin,end,map,flags);
		if(!res) return false;
		m.assign(begin,end,map);
		return true;
	}
	///
	/// Match an expression \a r against text \a s, return true
	/// if found and store matched patters in \a m
	///

	template<typename Regex>
	bool regex_match(std::string const &s,smatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		bool res = r.match(s.c_str(),s.c_str()+s.size(),map,flags);
		if(!res) return false;
		m.assign(s.begin(),s.end(),map);
		return true;
	}
	///
	/// Match an expression \a r against text \a s, return true
	/// if found and store matched patters in \a m
	///
	
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

	///
	/// Search an expression \a r in text in rage [\a begin, \a end). Return true if found,
	/// and store matched subexpressions in  \a m
	/// 
	template<typename Regex>
	bool regex_search(char const *begin,char const *end,cmatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		bool res = r.search(begin,end,map,flags);
		if(!res) return false;
		m.assign(begin,end,map);
		return true;
	}

	///
	/// Search an expression \a r in text \a s. Return true if found,
	/// and store matched subexpressions in  \a m
	/// 
	template<typename Regex>
	bool regex_search(std::string const &s,smatch &m, Regex const &r,int flags = 0)
	{
		std::vector<std::pair<int,int> > map;
		bool res = r.search(s.c_str(),s.c_str()+s.size(),map,flags);
		if(!res) return false;
		m.assign(s.begin(),s.end(),map);
		return true;
	}
	
	///
	/// Search an expression \a r in text \a s. Return true if found,
	/// and store matched subexpressions in  \a m
	/// 
	
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

	///
	/// Match an expression \a r against text in range [\a begin, \a end ), return true if matched
	///
	template<typename Regex>
	bool regex_match(char const *begin,char const *end, Regex const &r,int flags = 0)
	{
		return r.match(begin,end,flags);
	}
	///
	/// Match an expression \a r against text \a s, return true if matched
	///

	template<typename Regex>
	bool regex_match(std::string const &s, Regex const &r,int flags = 0)
	{
		return r.match(s.c_str(),s.c_str()+s.size(),flags);
	}
	///
	/// Match an expression \a r against text \a s, return true if matched
	///
	
	template<typename Regex>
	bool regex_match(char const *s, Regex const &r,int flags = 0)
	{
		return r.match(s,s+strlen(s),flags);
	}
	
	///
	/// Search an expression \a r against text in range [\a begin, \a end ), return true if found
	///
	template<typename Regex>
	bool regex_search(char const *begin,char const *end, Regex const &r,int flags = 0)
	{
		return r.search(begin,end,flags);
	}

	///
	/// Search an expression \a r against text \a s, return true if found
	///
	template<typename Regex>
	bool regex_search(std::string const &s, Regex const &r,int flags = 0)
	{
		return r.search(s.c_str(),s.c_str()+s.size(),flags);
	}
	
	///
	/// Search an expression \a r against text \a s, return true if found
	///
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
