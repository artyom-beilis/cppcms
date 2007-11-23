#ifndef _URL_H
#define _URL_H

#include <map>
#include <vector>
#include <boost/regex.hpp>
#include <boost/signals.hpp>
#include <boost/bind.hpp>

// Some defines:

#define BIND boost::bind

#define $0 _9
#define $1 _1
#define $2 _2
#define $3 _3
#define $4 _4
#define $5 _5
#define $6 _6
#define $7 _7
#define $8 _8


class Worker_Thread;
using std::string;

typedef boost::signal<void(string,string,string,string,
			   string,string,string,string,
			   string)> callback_signal_t;
typedef callback_signal_t::slot_type callback_t;

class URL_Parser;

struct URL_Def {
	boost::regex pattern;
	enum { ID, CALLBACK , URL } type;
	int id;
	URL_Parser *url;
	callback_signal_t callback;
};

class URL_Parser {
	int size;
	int filled;
	URL_Def *patterns;
	Worker_Thread *worker;
	boost::cmatch result;
	void set_regex(char const *r);
public:
	static const int default_size = 32;
	static const int not_found=-1;
	static const int ok=0;
	void reserve(int n) { 
		if (!patterns)
			patterns = new URL_Def [n];
		size=n;
		filled=0;
	};
	URL_Parser() { patterns = NULL;};
	URL_Parser(Worker_Thread * w) { worker=w; patterns = NULL;};
	URL_Parser(Worker_Thread * w,int size) { 
		worker=w;
		reserve(size);
	};
	URL_Parser(int size) { 
		worker=NULL;
		reserve(size);
	};
	~URL_Parser() { delete [] patterns; };
	void add(char const *exp,int id);
	void add(char const *exp,URL_Parser &url);
	void add(char const *exp,callback_t callback);
	void init(Worker_Thread *w) { worker=w; };
	int parse();
	int parse(string &s);
	std::string operator[](int);
};


#endif /* _URL_H */
