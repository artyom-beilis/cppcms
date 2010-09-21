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
#define CPPCMS_SOURCE
#include <cppcms/util.h>
#include "http_protocol.h"
#include <stdio.h>
#include "md5.h"

namespace cppcms {
namespace util {

std::string escape(std::string const &s)
{
	std::string content;
	unsigned i,len=s.size();
	content.reserve(len*3/2);
	for(i=0;i<len;i++) {
		char c=s[i];
		switch(c){
			case '<': content+="&lt;"; break;
			case '>': content+="&gt;"; break;
			case '&': content+="&amp;"; break;
			case '\"': content+="&quot;"; break;
			default: content+=c;
		}
	}
	return content;
}

std::string urlencode(std::string const &s)
{
	std::string content;
	unsigned i,len=s.size();
	content.reserve(3*len);
	for(i=0;i<len;i++){
		char c=s[i];
		if(	('a'<=c && c<='z')
			|| ('A'<=c && c<='Z')
			|| ('0'<=c && c<='9'))
		{
			content+=c;
		}
		else {
			switch(c) {
				case '-':
				case '_':
				case '.':
				case '~':
					content+=c;
					break;
				default:
				{
					char buf[4];
#ifdef CPPCMS_HAVE_SNPRINTF	
					snprintf(buf,sizeof(buf),"%%%02x",(unsigned)(c));
#else
					sprintf(buf,"%%%02x",(unsigned)(c));
					// Should be OK for this range
#endif					
					content.append(buf,3);
					
				}
			};
		}
	};
	return content;
}


std::string urldecode(std::string const &s)
{
	return urldecode(s.c_str(),s.c_str()+s.size());
}
// TODO: Find correct RFC for proprer decoding
std::string urldecode(char const *begin,char const *end)
{
	std::string result;
	result.reserve(end-begin);
	for(;begin<end;begin++) {
		char c=*begin;
		switch(c) {
		case '+': result+=' ';
			break;
		case '%':
			if(end-begin >= 3 && http::protocol::xdigit(begin[1]) && http::protocol::xdigit(begin[2])) {
				char buf[3]={begin[1],begin[2],0};
				int value;
				sscanf(buf,"%x",&value);
				result+=char(value);
				begin+=2;
			}
			break;
		default:
			result+=c;
		
		}
	}
	return result;
}

std::string md5(std::string const &in)
{
	using namespace cppcms::impl;
	unsigned char data[16];
	md5_state_t state;
	md5_init(&state);
	md5_append(&state,reinterpret_cast<unsigned const char *>(in.c_str()),in.size());
	md5_finish(&state,data);
	return std::string(reinterpret_cast<char *>(data),16);
}

std::string md5hex(std::string const &in)
{
	using namespace cppcms::impl;
	unsigned char data[16];
	md5_state_t state;
	md5_init(&state);
	md5_append(&state,reinterpret_cast<unsigned const char *>(in.c_str()),in.size());
	md5_finish(&state,data);
	char buf[33]={0};
	for(int i=0;i<16;i++) {
		unsigned val=data[i];
		sprintf(buf+i*2,"%02x",val);
	}
	return std::string(buf,32);
}



} // util
} // util
