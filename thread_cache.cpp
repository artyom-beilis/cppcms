#define CPPCMS_SOURCE
#include "config.h"
#include "thread_cache.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/thread.hpp>
#   include <boost/format.hpp>
#else // Internal Boost
#   include <cppcms_boost/thread.hpp>
#   include <cppcms_boost/format.hpp>
    namespace boost = cppcms_boost;
#endif

using namespace std;

namespace cppcms {
namespace impl {

class thread_cache : public base_cache {
	boost::mutex lru_mutex;
	boost::shared_mutex access_lock;
	struct container {
		string data;
		typedef std::map<string,container>::iterator pointer;
		list<pointer>::iterator lru;
		list<multimap<string,pointer>::iterator> triggers;
		multimap<time_t,pointer>::iterator timeout;
	};
	typedef container::pointer pointer;
	std::map<string,container> primary;
	multimap<string,pointer> triggers;
	typedef multimap<string,pointer>::iterator triggers_ptr;
	multimap<time_t,pointer> timeout;
	typedef multimap<time_t,pointer>::iterator timeout_ptr;
	list<pointer> lru;
	typedef list<pointer>::iterator lru_ptr;
	unsigned limit;

	string const *get(string const &key,set<string> *triggers);
	void delete_node(pointer p);

public:
	thread_cache(unsigned pages=0) : limit(pages) {}
	void set_size(unsigned l) { limit=l; };
	virtual bool fetch(string const &key,string &a,std::set<std::string> *tags);
	virtual void rise(string const &trigger);
	virtual void clear();
	virtual void stats(unsigned &keys,unsigned &triggers);
	virtual void store(string const &key,std::string const &a,set<string> const &triggers,time_t timeout);
	virtual ~thread_cache();
}; // thread cache


thread_cache::~thread_cache()
{
}

std::string const *thread_cache::get(string const &key,set<string> *triggers)
{
	pointer p;
	time_t now;
	time(&now);
	if((p=primary.find(key))==primary.end() || p->second.timeout->first < now) {
		return NULL;
	}
	if(triggers) {
		list<triggers_ptr>::iterator tp;
		for(tp=p->second.triggers.begin();tp!=p->second.triggers.end();tp++) {
			triggers->insert((*tp)->first);
		}
	}
	{
		boost::unique_lock<boost::mutex> lock(lru_mutex);
		lru.erase(p->second.lru);
		lru.push_front(p);
		p->second.lru=lru.begin();
	}
	return &(p->second.data);
}


bool thread_cache::fetch(string const &key,std::string &a,set<string> *tags)
{
	boost::shared_lock<boost::shared_mutex> lock(access_lock);
	string const *r=get(key,tags);
	if(!r) return false;
	a = *r;
	return true;
}

void thread_cache::clear()
{
	boost::unique_lock<boost::shared_mutex> lock(access_lock);
	timeout.clear();
	lru.clear();
	primary.clear();
	triggers.clear();
}
void thread_cache::stats(unsigned &keys,unsigned &triggers)
{
	boost::shared_lock<boost::shared_mutex> lock(access_lock);
	keys=primary.size();
	triggers=this->triggers.size();
}

void thread_cache::rise(string const &trigger)
{
	boost::unique_lock<boost::shared_mutex> lock(access_lock);
	pair<triggers_ptr,triggers_ptr> range=triggers.equal_range(trigger);
	triggers_ptr p;
	list<pointer> kill_list;
	for(p=range.first;p!=range.second;p++) {
		kill_list.push_back(p->second);
	}
	list<pointer>::iterator lptr;

	for(lptr=kill_list.begin();lptr!=kill_list.end();lptr++) {
		delete_node(*lptr);
	}
}

void thread_cache::store(string const &key,std::string const &a,set<string> const &triggers_in,time_t timeout_in)
{
	boost::unique_lock<boost::shared_mutex> lock(access_lock);
	pointer main;
	main=primary.find(key);
	if(main==primary.end() && primary.size()>=limit && limit>0) {
		time_t now;
		time(&now);
		if(timeout.begin()->first<now) {
			main=timeout.begin()->second;
		}
		else {
			main=lru.back();
		}
	}
	if(main!=primary.end())
		delete_node(main);
	pair<pointer,bool> res=primary.insert(pair<string,container>(key,container()));
	main=res.first;
	container &cont=main->second;
	cont.data=a;
	lru.push_front(main);
	cont.lru=lru.begin();
	cont.timeout=timeout.insert(pair<time_t,pointer>(timeout_in,main));
	if(triggers_in.find(key)==triggers_in.end()){
		cont.triggers.push_back(triggers.insert(pair<string,pointer>(key,main)));
	}
	set<string>::const_iterator si;
	for(si=triggers_in.begin();si!=triggers_in.end();si++) {
		cont.triggers.push_back(triggers.insert(pair<string,pointer>(*si,main)));
	}
}

void thread_cache::delete_node(pointer p)
{
	lru.erase(p->second.lru);
	timeout.erase(p->second.timeout);
	list<triggers_ptr>::iterator i;
	for(i=p->second.triggers.begin();i!=p->second.triggers.end();i++) {
		triggers.erase(*i);
	}
	primary.erase(p);
}

intrusive_ptr<base_cache> thread_cache_factory(unsigned items)
{
	return new thread_cache(items);
}


} // impl 
} // cppcms



