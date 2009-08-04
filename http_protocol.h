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
			case '\r':
			case '\n':
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
