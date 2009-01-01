#ifndef CACHE_IFACE_H
#define CACHE_IFACE_H

#include <string>
#include <set>
#include "archive.h"
#include "base_cache.h"

namespace cppcms {

using namespace std;

class worker_thread;
class cache_iface {
	worker_thread *cms;
	set<string> triggers;
public:
	void reset() { triggers.clear(); };
	cache_iface(worker_thread *w) : cms (w) {};
	bool fetch_page(string const &key);
	void store_page(string const &key,int timeout=-1);
	void rise(string const &trigger);
	void add_trigger(string const &trigger);
	bool fetch_frame(string const &key,string &result,bool notriggers=false);
	void store_frame(string const &key,
			 string const &frame,
			 set<string> const &triggers=set<string>(),
			 int timeout=-1);
	bool fetch_data(string const &key,serializable &data,bool notriggers=false);
	void store_data(string const &key,serializable const &data,
			set<string> const &triggers=set<string>(),
			int timeout=-1);
	void clear();
	bool stats(unsigned &keys,unsigned &triggers);

};

void deflate(string const &text,ostream &stream,long level,long length);
string deflate(string const &text,long level,long length);

class cache_factory {
public:
	virtual base_cache *get() const { return NULL; };
	virtual void del(base_cache *p) const { };
	virtual ~cache_factory() {};
};


}

#endif
