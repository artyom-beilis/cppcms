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
#ifndef CPPCMS_STRING_KEY_H
#define CPPCMS_STRING_KEY_H

#include <string>
#include <string.h>
#include <algorithm>
#include <stdexcept>
#include <ostream>

namespace cppcms {
	class string_key {
	public:

		typedef char const *const_iterator;

		static const size_t npos = -1;
		
		string_key() : 
			begin_(0),
			end_(0)
		{
		}

		string_key(char const *key) :
			begin_(0),
			end_(0),
			key_(key)
		{
		}
		string_key(std::string const &key) :
			begin_(0),
			end_(0),
			key_(key)
		{
		}

		size_t size() const
		{
			return end() - begin();
		}
		size_t length() const
		{
			return size();
		}
		void clear()
		{
			begin_ = end_ = 0;
			key_.clear();
		}
		bool empty() const
		{
			return end() == begin();
		}

		size_t find(char c,size_t pos = 0) const
		{
			size_t s = size();
			if(pos >= s)
				return npos;
			char const *p=begin() + pos;
			while(pos <= s && *p!=c) {
				pos++;
				p++;
			}
			if(pos >= s)
				return npos;
			return pos;
		}

		string_key substr(size_t pos = 0,size_t n=npos) const
		{
			string_key tmp = unowned_substr(pos,n);
			return string_key(std::string(tmp.begin(),tmp.end()));
		}
		string_key unowned_substr(size_t pos = 0,size_t n=npos) const
		{
			if(pos >= size()) {
				return string_key();
			}
			char const *p=begin() + pos;
			char const *e=end();
			if(n > size_t(e-p)) {
				return string_key(p,e);
			}
			else {
				return string_key(p,p+n);
			}
		}

		char const &operator[](size_t n) const
		{
			return *(begin() + n);
		}
		char const &at(size_t n) const
		{
			if(n > size())
				throw std::out_of_range("cppcms::string_key::at() range error");
			return *(begin() + n);
		}

		
		static string_key unowned(std::string const &v) 
		{
			return string_key(v.c_str(),v.c_str()+v.size());
		}
		static string_key unowned(char const *str) 
		{
			char const *end = str;
			while(*end)
				end++;
			return string_key(str,end);
		}
		static string_key unowned(char const *begin,char const *end) 
		{
			return string_key(begin,end);
		}

		char const *begin() const
		{
			if(begin_)
				return begin_;
			return key_.c_str();
		}
		char const *end() const
		{
			if(begin_)
				return end_;
			return key_.c_str() + key_.size();
		}
		bool operator<(string_key const &other) const
		{
			return std::lexicographical_compare(	begin(),end(),
								other.begin(),other.end(),
								std::char_traits<char>::lt);
		}
		bool operator>(string_key const &other) const
		{
			return other < *this;
		}
		bool operator>=(string_key const &other) const
		{
			return !(*this < other);
		}
		bool operator<=(string_key const &other) const
		{
			return !(*this > other);
		}
		bool operator==(string_key const &other) const
		{
			return (end() - begin() == other.end() - other.begin())
				&& memcmp(begin(),other.begin(),end()-begin()) == 0;
		}
		bool operator!=(string_key const &other) const
		{
			return !(*this!=other);
		}

		char const *data() const
		{
			return begin();
		}

		std::string str() const
		{
			if(begin_)
				return std::string(begin_,end_-begin_);
			else
				return key_;
		}
		operator std::string() const
		{
			return str();
		}
	private:
		string_key(char const *b,char const *e) :
			begin_(b),
			end_(e)
		{
		}

		char const *begin_;
		char const *end_;
		std::string key_;
	};

	inline std::ostream &operator<<(std::ostream &out,string_key const &s)
	{
		out.write(s.data(),s.size());
		return out;
	}

	inline bool operator==(string_key const &l,char const *r)
	{
		return l==string_key::unowned(r);
	}

	inline bool operator==(char const *l,string_key const &r)
	{
		return string_key::unowned(l) == r;
	}

	inline bool operator==(string_key const &l,std::string const &r)
	{
		return l==string_key::unowned(r);
	}

	inline bool operator==(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) == r;
	}

	inline bool operator!=(string_key const &l,char const *r)
	{
		return l!=string_key::unowned(r);
	}

	inline bool operator!=(char const *l,string_key const &r)
	{
		return string_key::unowned(l) != r;
	}

	inline bool operator!=(string_key const &l,std::string const &r)
	{
		return l!=string_key::unowned(r);
	}

	inline bool operator!=(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) != r;
	}
	inline bool operator<=(string_key const &l,char const *r)
	{
		return l<=string_key::unowned(r);
	}

	inline bool operator<=(char const *l,string_key const &r)
	{
		return string_key::unowned(l) <= r;
	}

	inline bool operator<=(string_key const &l,std::string const &r)
	{
		return l<=string_key::unowned(r);
	}

	inline bool operator<=(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) <= r;
	}
	inline bool operator>=(string_key const &l,char const *r)
	{
		return l>=string_key::unowned(r);
	}

	inline bool operator>=(char const *l,string_key const &r)
	{
		return string_key::unowned(l) >= r;
	}

	inline bool operator>=(string_key const &l,std::string const &r)
	{
		return l>=string_key::unowned(r);
	}

	inline bool operator>=(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) >= r;
	}


	inline bool operator<(string_key const &l,char const *r)
	{
		return l<string_key::unowned(r);
	}

	inline bool operator<(char const *l,string_key const &r)
	{
		return string_key::unowned(l) < r;
	}

	inline bool operator<(string_key const &l,std::string const &r)
	{
		return l<string_key::unowned(r);
	}

	inline bool operator<(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) < r;
	}
	inline bool operator>(string_key const &l,char const *r)
	{
		return l>string_key::unowned(r);
	}

	inline bool operator>(char const *l,string_key const &r)
	{
		return string_key::unowned(l) > r;
	}

	inline bool operator>(string_key const &l,std::string const &r)
	{
		return l>string_key::unowned(r);
	}

	inline bool operator>(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) > r;
	}

}

#endif
