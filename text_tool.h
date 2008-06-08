#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <string>

namespace cppcms {
	namespace texttool {
		using std::string;
		void text2html(string const &s,string &out);
		void text2url(string const &s,string &out);
	};
};


#endif
