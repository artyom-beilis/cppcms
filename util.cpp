#define CPPCMS_SOURCE
#include "util.h"
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
					snprintf(buf,sizeof(buf),"%%%02x",(unsigned)(c));
					content.append(buf,3);
				}
			};
		}
	};
	return content;
}

} // util
} // util
