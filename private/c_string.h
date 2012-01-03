///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_C_STRING_H
#define CPPCMS_C_STRING_H

#include <string.h>
#include <string>

namespace cppcms {
	namespace xss {
		
		namespace details {
	  
			class c_string {
			public:
				
				typedef char const *const_iterator;
				
				char const *begin() const
				{
					return begin_;
				}
				
				char const *end() const
				{
					return end_;
				}
				
				c_string(char const *s) 
				{
					begin_=s;
					end_=s+strlen(s);
				}

				c_string(char const *b,char const *e) : begin_(b), end_(e) {}

				c_string() : begin_(0),end_(0) {}

				bool compare(c_string const &other) const
				{
					return std::lexicographical_compare(begin_,end_,other.begin_,other.end_,std::char_traits<char>::lt);
				}

				bool icompare(c_string const &other) const
				{
					return std::lexicographical_compare(begin_,end_,other.begin_,other.end_,ilt);
				}

				explicit c_string(std::string const &other)
				{
					container_ = other;
					begin_ = container_.c_str();
					end_ = begin_ + container_.size();
				}
				c_string(c_string const &other)
				{
					if(other.begin_ == other.end_) {
						begin_ = end_ = 0;
					}
					else if(other.container_.empty()) {
						begin_ = other.begin_;
						end_ = other.end_;
					}
					else {
						container_ = other.container_;
						begin_ = container_.c_str();
						end_ = begin_ + container_.size();
					}
				}
				c_string const &operator=(c_string const &other)
				{
					if(other.begin_ == other.end_) {
						begin_ = end_ = 0;
					}
					else if(other.container_.empty()) {
						begin_ = other.begin_;
						end_ = other.end_;
					}
					else {
						container_ = other.container_;
						begin_ = container_.c_str();
						end_ = begin_ + container_.size();
					}
					return *this;
				}

			private:
				static bool ilt(char left,char right)
				{
					unsigned char l = tolower(left);
					unsigned char r = tolower(right);
					return l < r;
				}
				static char tolower(char c)
				{
					if('A' <= c && c<='Z')
						return c-'A' + 'a';
					return c;
				}
				char const *begin_;
				char const *end_;
				std::string container_;
			};
			
		}  // details
	} // xss
} // cppcms

#endif
