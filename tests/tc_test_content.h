#include <cppcms/view.h>
#include <string>
#include <vector>

namespace data {
	struct helper : public cppcms::base_content  {
		int x;
		int y;
	};
	struct master  : public cppcms::base_content {
		int integer;
		std::string text;
		helper h;
		typedef std::vector<int> integers_type;
		integers_type integers;
		std::string test_filter(std::string const &s)
		{
			std::string tmp = s;
			for(size_t i=0;i<tmp.size();i++) {
				switch(tmp[i]) {
				case 'a' : tmp[i]='b'; break;
				case 'b' : tmp[i]='a'; break;
				}
			}
			return tmp;
		}
		void (*call)();
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

	struct form_test : public master {
		cppcms::form f;
		cppcms::widgets::text t1;
		cppcms::widgets::text t2;
		form_test()
		{
			f.add(t1);
			f.add(t2);
			t1.message("msg");
			t1.help("help");
		}
	};
} // data
