#ifndef _URL_H
#define _URL_H

#include <map>
#include <vector>
#include <boost/regex.hpp>

struct URL_Def {
	char const *url;
	int id;
};

class Worker_Thread;

class URL_Parser {
	static std::vector<std::pair<boost::regex,int> > all_urls;
	Worker_Thread *worker;
	boost::cmatch result;
public:
	static void add(char const *name,int id);
	static void add(URL_Def *list);
	void init(Worker_Thread *);
	int parse();
	std::string operator[](int );
};


#endif /* _URL_H */
