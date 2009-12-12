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
	void print_all();
	bool debug_mode;
	int fd;

public:
	void set_debug_mode(int fd) { debug_mode=true; this->fd=fd; };
	thread_cache(unsigned pages=0) : limit(pages) {
		debug_mode=false;
	};
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
	if(debug_mode)	print_all();
	if((p=primary.find(key))==primary.end() || p->second.timeout->first < now) {
		if(debug_mode) {
			string res;
			if(p==primary.end()) {
				res=str(boost::format("Not found [%1%]\n") % key);
			}
			else {
				res=str(boost::format("Found [%1%] but timeout of %2% seconds\n")
					% key % (now - p->second.timeout->first));
			}
			write(fd,res.c_str(),res.size());
		}
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
	if(debug_mode){
		string res=(boost::format("Fetched [%1%] triggers:") % key).str();
		list<triggers_ptr>::iterator tp;
		for(tp=p->second.triggers.begin();
			tp!=p->second.triggers.end();tp++)
		{
			res+=(*tp)->first;
			res+=" ";
		}
		res+="\n";
		write(fd,res.c_str(),res.size());
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
	if(debug_mode)	print_all();
	pair<triggers_ptr,triggers_ptr> range=triggers.equal_range(trigger);
	triggers_ptr p;
	list<pointer> kill_list;
	for(p=range.first;p!=range.second;p++) {
		kill_list.push_back(p->second);
	}
	list<pointer>::iterator lptr;
	if(debug_mode){
		string out=str(boost::format("Trigger [%1%] dropping: ") % trigger);
		write(fd,out.c_str(),out.size());
	}

	for(lptr=kill_list.begin();lptr!=kill_list.end();lptr++) {
		if(debug_mode) {
			write(fd,(*lptr)->first.c_str(),(*lptr)->first.size());
			write(fd," ",1);
		}
		delete_node(*lptr);
	}
	if(debug_mode)
		write(fd,"\n",1);
}

void thread_cache::store(string const &key,std::string const &a,set<string> const &triggers_in,time_t timeout_in)
{
	boost::unique_lock<boost::shared_mutex> lock(access_lock);
	if(debug_mode)	print_all();
	pointer main;
	if(debug_mode) {
		string res;
		res=str(boost::format("Storing key [%1%], triggers:") % key);
		for(set<string>::iterator ps=triggers_in.begin(),pe=triggers_in.end();ps!=pe;ps++) {
			res+=*ps;
			res+=" ";
		}
		res+="\n";
		write(fd,res.c_str(),res.size());
	}
	main=primary.find(key);
	if(main==primary.end() && primary.size()>=limit && limit>0) {
		if(debug_mode) {
			char const *msg="Not found, size limit\n";
			write(fd,msg,strlen(msg));
		}
		time_t now;
		time(&now);
		if(timeout.begin()->first<now) {
			main=timeout.begin()->second;
			if(debug_mode) {
				string res;
				res=str(boost::format("Deleting timeout node [%1%] with "
							"delta  of %2% seconds\n") % main->first
							% (now - main->second.timeout->first));
				write(fd,res.c_str(),res.size());
			}
		}
		else {
			main=lru.back();
			if(debug_mode) {
				string res;
				res=str(boost::format("Deleting LRU [%1%]\n") % main->first);
				write(fd,res.c_str(),res.size());
			}
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

void thread_cache::print_all()
{
	string res;
	res+="Printing stored keys\n";
	unsigned N_triggers=0;
	unsigned N_keys=0;
	time_t now;
	time(&now);
	for(pointer p=primary.begin();p!=primary.end();p++) {
		N_keys++;
		res+=str(boost::format("%1%: timeount in %2% sec, triggers:") % p->first
			% (p->second.timeout->first - now));
		for(list<triggers_ptr>::iterator p1=p->second.triggers.begin(),
			p2=p->second.triggers.end();
			p2!=p1;p1++)
		{
			N_triggers++;
			res+=(*p1)->first;
			res+=" ";
		}
		res+="\n";
	}
	res+="LRU order:";
	for(list<pointer>::iterator pl=lru.begin();pl!=lru.end();pl++) {
		res+=(*pl)->first;
		res+=" ";
	}
	res+="\n";
	if(N_keys!=timeout.size() || N_keys!=lru.size() || N_triggers!=triggers.size()){
		res+=str(boost::format("Internal error #prim=%1%, #lru=%2%, "
				"#prim.triggers=%3% #triggers=%4%\n")
				% N_keys % lru.size() % N_triggers % triggers.size());
	}
	else {
		res+=str(boost::format("#Keys=%1% #Triggers=%2%\n") % N_keys % N_triggers);
	}
	write(fd,res.c_str(),res.size());
}

intrusive_ptr<base_cache> thread_cache_factory(unsigned items)
{
	return new thread_cache(items);
}


} // impl 
} // cppcms



