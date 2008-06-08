#ifndef BASE_CACHE_H
#define BASE_CACHE_H

#include <string>
#include <set>
#include "archive.h"

namespace cppcms {

using namespace std;

class base_cache {
public:
	virtual bool fetch_page(string const &key,string &output,bool gzip);
	virtual bool fetch(string const &key,archive &a,set<string> &tags);
	virtual void rise(string const &trigger);
	virtual void clear();
	virtual void store(string const &key,set<string> const &triggers,time_t timeout,archive const &a);
	virtual void stats(unsigned &keys,unsigned &triggers);
	virtual ~base_cache();
};

}

#endif
