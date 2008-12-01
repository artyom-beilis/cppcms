#ifndef TCP_CHACHE_H
#define TCP_CHACHE_H
#include "base_cache.h"
#include "cache_interface.h"
#include <string>

namespace cppcms {

using namespace std;

class messenger;
struct tcp_operation_header;

class tcp_cache : public base_cache {
	messenger *tcp;
	int conns;
	messenger &get(string const &key);
	void broadcast(tcp_operation_header &h,string &data);
public:
	tcp_cache(vector<string> const &ip_list,vector<int> const &port_list);

	virtual bool fetch_page(string const &key,string &output,bool gzip);
	virtual bool fetch(string const &key,archive &a,set<string> &tags);
	virtual void rise(string const &trigger);
	virtual void clear();
	virtual void stats(unsigned &keys,unsigned &triggers);
	virtual void store(string const &key,set<string> const &triggers,time_t timeout,archive const &a);
	virtual ~tcp_cache();
};

class tcp_cache_factory : public cache_factory {
	vector<string> ip;
	vector<int>   port;
public:
	tcp_cache_factory(vector<string> const &_ip,vector<int> const &_port) :
		ip(_ip),port(_port) {};
	virtual base_cache *get() const { return new tcp_cache(ip,port); };
	virtual void del(base_cache *p) const { delete p; };
};

}

#endif
