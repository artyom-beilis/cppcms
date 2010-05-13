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
#ifndef CPPCMS_HTTP_PROTOCOL_H
#define CPPCMS_HTTP_PROTOCOL_H

namespace cppcms {
namespace http { 
namespace protocol {


	inline bool separator(char c)
	{
		switch(c) {
		case '(':
		case ')':
		case '<':
		case '>':
		case '@':
		case ',':
		case ';':
		case ':':
		case '\\':
		case '"':
		case '/':
		case '[':
		case ']':
		case '?':
		case '=':
		case '{':
		case '}':
		case ' ':
		case '\t':
			return true;
		default:
			return false;
		}
	}
	
	inline char ascii_to_lower(char c)
	{
		if(c<'A' || c>'Z') return c;
		return c-'A'+'a';
	}

	template<typename It>
	It skip_ws(It p,It end)
	{
		while(p<end) {
			switch(*p) {
			case '\r': // Check for LWS (CRLF 1*( SP | HT))
				if(p+2 < end && *(p+1)=='\n' && (*(p+2)==' ' || *(p+2)=='\t')) {
					p+=2;
					break;
				}
				return p;
			case ' ':
			case '\t':
				break;
			default:
				return p;
			}
			p++;
		}
		return p;
	}
	template<typename It>
	It tocken(It begin,It end)
	{
		char c;
		while(begin < end && 0x20<=(c=*begin) && c<=0x7E && !separator(c))
			begin++;
		return begin;
	}
	
	template<typename It>
	std::string unquote(It &begin,It end)
	{
		It p=begin;
		std::string result;
		if(p>=end || *p++!='\"')
			return result;
		result.reserve(end-p);
		p++;
		while(p < end) {
			char c=*p++;
			if(c=='\"') {
				begin=p;
				return result;
			}
			else if(c=='\\' && p<end)
				result+= *p++;
			else
				result+=c;
		}
		result.clear();
		return result;
	}
	
	inline int compare(std::string const &left,std::string const &right)
	{
		size_t lsize=left.size();
		size_t rsize=right.size();
		for(size_t i=0;i<lsize && i<rsize;i++) {
			char cl=ascii_to_lower(left[i]);
			char cr=ascii_to_lower(right[i]);
			if(cl<cr) return -1;
			if(cl>cr) return 1;
			// if(cl==cr) continue
		}
		if(lsize<rsize)
			return -1;
		if(lsize>rsize)
			return 1;
		return 0;
	}

	inline std::string quote(std::string const &input)
	{
		std::string result;
		result.reserve(input.size());
		result+='"';
		for(std::string::const_iterator p=input.begin();p!=input.end();++p) {
			char c=*p;
			if(0<=c && c<' ') {
				result+='\\';
			}
			result+=c;
		}
		result+='"';
		return result;
	}

	bool inline is_prefix_of(char const *prefix,std::string const &s)
	{
		std::string::const_iterator p=s.begin(),e=s.end();
		p=skip_ws(p,e);
		char c;
		while((c=(*prefix++))!=0) {
			if(p==e)
				return false;
			if(ascii_to_lower(c)!=ascii_to_lower(*p++))
				return false;
		}
		return true;
	}
	bool inline xdigit(int c) { return ('0'<=c && c<='9') || ('a'<=c && c<='f') || ('A'<=c && c<='F'); }


} // protocol
} // http
} // cppcms


#endif
