#ifndef _TEXTSTREAM_H
#define _TEXTSTREAM_H

#include <string>

#include "global_config.h"

using namespace std;

#define TS_BUFFER_SIZE          1024
#define TS_INITAL_ALLOCATION    0x10000


class Text_Stream {
	int inital_alloc;
	char buffer[TS_BUFFER_SIZE];
	string text;
public:
	void reset(void)  {
		text.clear();
		text.reserve(inital_alloc);
	};
	Text_Stream() {
		inital_alloc=global_config.lval("performance.textalloc",
							TS_INITAL_ALLOCATION);
		reset();
	};
	~Text_Stream() {};
	void puts(char const *t) { text += t; };
	void putchar(char c) { text += c; };
	char const *get() { return text.c_str(); };
	int len() { return text.size(); };
	void printf(char *format, ...);
	void puts(string &str) { text += str; };
};

#endif /* _TEXTSTREAM_H */
