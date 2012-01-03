///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_FORMAT_NUMBER_H
#define CPPCMS_IMPL_FORMAT_NUMBER_H


#include <locale>
#include <sstream>
#include <iomanip>
#include <limits>
#include <string.h>

namespace cppcms {
	namespace impl {
		template<typename Value,bool is_integer=std::numeric_limits<Value>::is_integer>
		struct format_traits;
		
		template<typename Value>
		struct format_traits<Value,true> {
			static void format(Value v,char *output,size_t n)
			{
				char buf[std::numeric_limits<Value>::digits10 + 10];
				char *begin = buf;
				if(v < 0) {
					*begin ++ = '-';
				}
				if(v == 0) {
					*begin ++ ='0';
					*begin = 0;
				}
				else {
					char *p=begin;
					while(v != 0) {
						int digit = v % 10;
						v/=10;
						if(digit < 0)
							*p++='0' - digit;
						else
							*p++='0' + digit;
					}
					*p-- = 0;
					while(begin < p) {
						std::swap(*begin,*p);
						begin++;
						p--;
					}
				}
				strncpy(output,buf,n-1);
				output[n-1] = 0;
			}
		};
		template<typename Value>
		struct format_traits<Value,false> {
			static void format(Value v,char *output,size_t n)
			{
				std::ostringstream ss;
				ss.imbue(std::locale::classic());
				ss<<std::setprecision(std::numeric_limits<double>::digits10+1)<<v;
				strncpy(output,ss.str().c_str(),n-1);
				output[n-1] = 0;
			}
		};

		template<typename Value>
		void format_number(Value v,char *begin,size_t n)
		{
			format_traits<Value>::format(v,begin,n);
		}
	}
}
#endif
