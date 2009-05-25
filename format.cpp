#define CPPCMS_SOURCE
#include "format.h"

namespace cppcms { namespace util {

format_iterator::format_iterator(std::ostream &out,std::string const &format) :
	out_(out),
	current_(format.data()),
	end_(format.data()+format.size())
{
}

format_iterator::~format_iterator()
{
}

int format_iterator::write()
{
	while(current_!=end_) {
		char const *p=std::find(current_,end_,'%');
		if(p!=current_) {
			out_.write(current_,p-current_);
			current_=p;
			continue;
		}
		if(p+1<end_ && p[1]=='%') {
			out_.put('%');
			current_=p+2;
			continue;
		}
		if(p+2<end_ && '1'<=p[1] && p[1]<='9' && p[2]=='%') {
			current_=p+3;
			return p[1] - '0';
		}
		out_.put('%');
		current_++;
	}
	return end;
}


}}// cppcms::util
