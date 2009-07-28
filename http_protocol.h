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
	
	template<typename It>
	It tocken(It begin,It end)
	{
		It start=begin;
		char c;
		while(0x20<=(c=*begin++) && c<=0x7E && !separator(c))
			;
		return begin;
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


} // protocol
} // http
} // cppcms


#endif
