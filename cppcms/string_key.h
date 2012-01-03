///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
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

	///
	/// \brief This is a special object that may hold an std::string or
	/// alternatively reference to external (unowned) chunk of text
	///
	/// It is designed to be used for efficiency and reduce amount of
	/// memory allocations and copies.
	///
	/// It has interface that is roughly similar to the interface of std::string,
	/// but it does not provide a members that can mutate it or provide a NUL terminated
	/// string c_str().
	///
	class string_key {
	public:

		///
		/// Iterator type
		///
		typedef char const *const_iterator;

		///
		/// The last position of the character in the string
		///
		static const size_t npos = -1;
		
		///
		/// Default constructor - empty key
		///
		string_key() : 
			begin_(0),
			end_(0)
		{
		}

		///
		/// Create a new string copying the \a key
		///
		string_key(char const *key) :
			begin_(0),
			end_(0),
			key_(key)
		{
		}
		///
		/// Create a new string copying the \a key
		///
		string_key(std::string const &key) :
			begin_(0),
			end_(0),
			key_(key)
		{
		}
		///
		/// String size in bytes
		///
		size_t size() const
		{
			return end() - begin();
		}
		///
		/// Same as size()
		///
		size_t length() const
		{
			return size();
		}
		///
		/// Clear the string
		///
		void clear()
		{
			begin_ = end_ = 0;
			key_.clear();
		}
		///
		/// Check if the string is empty
		///
		bool empty() const
		{
			return end() == begin();
		}
		///
		/// Find first occurrence of a character \c in the string starting from
		/// position \a pos. Returns npos if not character found.
		///
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

		///
		/// Create a substring from this string starting from character \a pos of size at most \a n
		///
		string_key substr(size_t pos = 0,size_t n=npos) const
		{
			string_key tmp = unowned_substr(pos,n);
			return string_key(std::string(tmp.begin(),tmp.end()));
		}
		///
		/// Create a substring from this string starting from character \a pos of size at most \a n
		/// such that the memory is not copied but only reference by the created substring
		///
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
		
		///
		/// Get a character at position \a n
		///
		char const &operator[](size_t n) const
		{
			return *(begin() + n);
		}
		///
		/// Get a character at position \a n, if \a n is not valid position, throws std::out_of_range exception
		///
		char const &at(size_t n) const
		{
			if(n > size())
				throw std::out_of_range("cppcms::string_key::at() range error");
			return *(begin() + n);
		}

		///
		/// Create a string from \a v without copying the memory. \a v should remain valid
		/// as long as this object is used
		///	
		static string_key unowned(std::string const &v) 
		{
			return string_key(v.c_str(),v.c_str()+v.size());
		}
		///
		/// Create a string from \a str without copying the memory. \a str should remain valid
		/// as long as this object is used
		///	
		static string_key unowned(char const *str) 
		{
			char const *end = str;
			while(*end)
				end++;
			return string_key(str,end);
		}
		///
		/// Create a string from \a characters at rang [begin,end) without copying the memory.
		/// The range should remain valid as long as this object is used
		///	
		static string_key unowned(char const *begin,char const *end) 
		{
			return string_key(begin,end);
		}

		///
		/// Get a pointer to the first character in the string
		///
		char const *begin() const
		{
			if(begin_)
				return begin_;
			return key_.c_str();
		}
		///
		/// Get a pointer to the one past last character in the string
		///
		char const *end() const
		{
			if(begin_)
				return end_;
			return key_.c_str() + key_.size();
		}
		///
		/// Compare two strings
		///
		bool operator<(string_key const &other) const
		{
			return std::lexicographical_compare(	begin(),end(),
								other.begin(),other.end(),
								std::char_traits<char>::lt);
		}
		///
		/// Compare two strings
		///
		bool operator>(string_key const &other) const
		{
			return other < *this;
		}
		///
		/// Compare two strings
		///
		bool operator>=(string_key const &other) const
		{
			return !(*this < other);
		}
		///
		/// Compare two strings
		///
		bool operator<=(string_key const &other) const
		{
			return !(*this > other);
		}
		///
		/// Compare two strings
		///
		bool operator==(string_key const &other) const
		{
			return (end() - begin() == other.end() - other.begin())
				&& memcmp(begin(),other.begin(),end()-begin()) == 0;
		}
		///
		/// Compare two strings
		///
		bool operator!=(string_key const &other) const
		{
			return !(*this!=other);
		}

		///
		/// Get the pointer to the first character in the string. Note it should not be NUL terminated
		///
		char const *data() const
		{
			return begin();
		}

		///
		/// Create std::string from the key
		///
		std::string str() const
		{
			if(begin_)
				return std::string(begin_,end_-begin_);
			else
				return key_;
		}
		///
		/// Convert the key to the std::string
		///
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

	///
	/// Write the string to the stream
	///
	inline std::ostream &operator<<(std::ostream &out,string_key const &s)
	{
		out.write(s.data(),s.size());
		return out;
	}

	///
	/// Compare two strings
	///
	inline bool operator==(string_key const &l,char const *r)
	{
		return l==string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator==(char const *l,string_key const &r)
	{
		return string_key::unowned(l) == r;
	}

	///
	/// Compare two strings
	///
	inline bool operator==(string_key const &l,std::string const &r)
	{
		return l==string_key::unowned(r);
	}


	///
	/// Compare two strings
	///
	inline bool operator==(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) == r;
	}

	///
	/// Compare two strings
	///
	inline bool operator!=(string_key const &l,char const *r)
	{
		return l!=string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator!=(char const *l,string_key const &r)
	{
		return string_key::unowned(l) != r;
	}

	///
	/// Compare two strings
	///
	inline bool operator!=(string_key const &l,std::string const &r)
	{
		return l!=string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator!=(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) != r;
	}
	///
	/// Compare two strings
	///
	inline bool operator<=(string_key const &l,char const *r)
	{
		return l<=string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator<=(char const *l,string_key const &r)
	{
		return string_key::unowned(l) <= r;
	}

	///
	/// Compare two strings
	///
	inline bool operator<=(string_key const &l,std::string const &r)
	{
		return l<=string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator<=(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) <= r;
	}
	///
	/// Compare two strings
	///
	inline bool operator>=(string_key const &l,char const *r)
	{
		return l>=string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator>=(char const *l,string_key const &r)
	{
		return string_key::unowned(l) >= r;
	}

	///
	/// Compare two strings
	///
	inline bool operator>=(string_key const &l,std::string const &r)
	{
		return l>=string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator>=(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) >= r;
	}


	///
	/// Compare two strings
	///
	inline bool operator<(string_key const &l,char const *r)
	{
		return l<string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator<(char const *l,string_key const &r)
	{
		return string_key::unowned(l) < r;
	}

	///
	/// Compare two strings
	///
	inline bool operator<(string_key const &l,std::string const &r)
	{
		return l<string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator<(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) < r;
	}
	///
	/// Compare two strings
	///
	inline bool operator>(string_key const &l,char const *r)
	{
		return l>string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator>(char const *l,string_key const &r)
	{
		return string_key::unowned(l) > r;
	}

	///
	/// Compare two strings
	///
	inline bool operator>(string_key const &l,std::string const &r)
	{
		return l>string_key::unowned(r);
	}

	///
	/// Compare two strings
	///
	inline bool operator>(std::string const &l,string_key const &r)
	{
		return string_key::unowned(l) > r;
	}

}

#endif
