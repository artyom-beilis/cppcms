#ifndef _URL_H
#define _URL_H

#include <map>
#include <memory>
#include <vector>
#include <boost/regex.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "config.h"

// Some defines:

#if defined(HAVE_DOLLAR_SIGN) && !defined(NO_DEPRICATED)

#define $0 _9
#define $1 _1
#define $2 _2
#define $3 _3
#define $4 _4
#define $5 _5
#define $6 _6
#define $7 _7
#define $8 _8

#endif

namespace cppcms {

class worker_thread;

using std::string;


typedef boost::function<void(string,string,string,string,
			   string,string,string,string,
			   string)> callback_t;
class  url_parser;

struct url_def {
	boost::regex pattern;
	enum { ID, CALLBACK , URL } type;
	int id;
	url_parser *url;
	callback_t callback;
};

class url_parser {
	std::vector<url_def>patterns;
	worker_thread *worker;
	boost::cmatch result;
	void set_regex(char const *r);
public:
	static const int not_found=-1;
	static const int ok=0;
	url_parser() {};
	url_parser(worker_thread * w) { worker=w;};
	~url_parser();
	void add(char const *exp,int id);
	void add(char const *exp,url_parser &url);
	void add(char const *exp,callback_t callback);
	void init(worker_thread *w) { worker=w; };
	int parse();
	int parse(string &s);
	std::string operator[](int);
};

} // Namespace cppcms

#endif /* _URL_H */
