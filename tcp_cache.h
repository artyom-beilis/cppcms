#ifndef TCP_CHACHE_H
#define TCP_CHACHE_H
#include "base_cache.h"
#include "cache_interface.h"
#include <string>

namespace cppcms {

using namespace std;

class messenger;

class tcp_cache : public base_cache {
	messenger *tcp;
public:
	tcp_cache(string ip,int port);
	virtual bool fetch_page(string const &key,string &output,bool gzip);
	virtual bool fetch(string const &key,archive &a,set<string> &tags);
	virtual void rise(string const &trigger);
	virtual void clear();
	virtual void stats(unsigned &keys,unsigned &triggers);
	virtual void store(string const &key,set<string> const &triggers,time_t timeout,archive const &a);
	virtual ~tcp_cache();
};

class tcp_cache_factory : public cache_factory {
	string ip;
	int port;
public:
	tcp_cache_factory(string _ip,int _port) :
		ip(_ip),port(_port) {};
	virtual base_cache *get() const { return new tcp_cache(ip,port); };
	virtual void del(base_cache *p) const { delete p; };
};

}

#endif
