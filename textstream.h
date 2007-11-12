#ifndef _TEXTSTREAM_H
#define _TEXTSTREAM_H

#include <string>

#include "config.h"

using namespace std;

class Text_Stream {

	char buffer[TS_BUFFER_SIZE];
	string text;
public:
	void reset(void)  {
		text.clear();
		text.reserve(INITAL_ALLOCATION);
	};
	Text_Stream() { reset(); };
	~Text_Stream() {};
	void puts(char const *t) { text += t; };
	void putchar(char c) { text += c; };
	char const *get() { return text.c_str(); };
	int len() { return text.size(); };
	void printf(char *format, ...);
	void puts(string &str) { text += str; };
};

#endif /* _TEXTSTREAM_H */
