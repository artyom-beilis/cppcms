#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <string>

using std::string;

class Text_Tool {
	// State definitions:
	enum { NOTHING, QUOTE, CODE, UL, OL, P, FINISH };
	enum { L_BLANK, L_TEXT, L_H, L_QUOTE, L_CODE, L_UL, L_OL ,L_EOF	};
	int state;
	int input;
	size_t ptr;
	string content;
	int header_level;
	void getline(string &s);
	void init();
	void to_html(string s);
	void basic_to_html(string s);
public:
	void markdown2html(char const *in,string &out);
	void markdown2html(string &in,string &out) { markdown2html(in.c_str(),out);};
	void text2html(char const *s,string &);
	void text2html(string &s,string &out) {	text2html(s.c_str(),out);};
	void text2url(char const *s,string &);
};


#endif
