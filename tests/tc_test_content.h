#include <cppcms/view.h>
#include <string>
#include <vector>

namespace data {
	struct master  : public cppcms::base_content {
		int integer;
		std::string text;
		typedef std::vector<int> integers_type;
		integers_type integers;
	};

	struct foo : public master {
		double real;
	};

	struct bar : public master {
		typedef std::vector<std::string> set_type;
		set_type set;
		std::string text2html(std::string const &input)
		{
			std::string output;
			for(unsigned i=0;i<input.size();i++) {
				switch(input[i]) {
				case '<':
					output+="&lt;";
					break;
				case '>':
					output+="&gt;";
					break;
				case '&':
					output+="&amp;";
					break;
				case '\r':
					break;
				case '\n':
					output+="<br />\n";
					break;
				default:
					output+=input[i];
				}
			}
			return output;
		}
	};
} // data
