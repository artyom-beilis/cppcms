#define CPPCMS_SOURCE
#include "util.h"
#include "http_protocol.h"
#include <stdio.h>

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
#ifdef HAVE_SNPRINTF	
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
} // util
} // util
