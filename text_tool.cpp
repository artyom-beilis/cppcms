#include "text_tool.h"
using namespace std;
namespace cppcms{
namespace texttool {


void text2html(string const &s,string &content)
{
	unsigned i;
	for(i=0;i<s.size();i++) {
		char c=s[i];
		switch(c){
			case '<': content+="&lt;"; break;
			case '>': content+="&gt;"; break;
			case '&': content+="&amp;"; break;
			case '\"': content+="&quot;"; break;
			default: content+=c;
		}
	}
}

}
}
