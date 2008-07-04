#include "thread_cache.h"
#include <boost/format.hpp>
#include <unistd.h>
#include "posix_mutex.h"

using boost::format;
using boost::str;

namespace cppcms {

thread_cache::~thread_cache()
{
	pthread_mutex_destroy(&lru_mutex);
	pthread_rwlock_destroy(&access_lock);
}

string *thread_cache::get(string const &key,set<string> *triggers)
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
		mutex_lock lock(lru_mutex);
		lru.erase(p->second.lru);
		lru.push_front(p);
		p->second.lru=lru.begin();
	}
	if(debug_mode){
		string res=str(boost::format("Fetched [%1%] triggers:") % key);
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

bool thread_cache::fetch_page(string const &key,string &out,bool gzip)
{
	rwlock_rdlock lock(access_lock);
	string *r=get(key,NULL);
	if(!r) return false;
	size_t size=r->size();
	size_t s;
	char const *ptr=r->c_str();
	if(size<sizeof(size_t) || (s=*(size_t const *)ptr)>size-sizeof(size_t))
		return false;
	if(!gzip){
		out.assign(ptr+sizeof(size_t),s);
	}
	else {
		ptr+=s+sizeof(size_t);
		size-=s+sizeof(size_t);
		if(size<sizeof(size_t) || (s=*(size_t const *)ptr)!=size-sizeof(size_t))
			return false;
		out.assign(ptr+sizeof(size_t),s);
	}
	return true;
}

bool thread_cache::fetch(string const  &key,archive &a,set<string> &tags)
{
	rwlock_rdlock lock(access_lock);
	string *r=get(key,&tags);
	if(!r) return false;
	a.set(*r);
	return true;
}

void thread_cache::clear()
{
	rwlock_wrlock lock(access_lock);
	timeout.clear();
	lru.clear();
	primary.clear();
	triggers.clear();
}
void thread_cache::stats(unsigned &keys,unsigned &triggers)
{
	rwlock_rdlock lock(access_lock);
	keys=primary.size();
	triggers=this->triggers.size();
}

void thread_cache::rise(string const &trigger)
{
	rwlock_wrlock lock(access_lock);
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

void thread_cache::store(string const &key,set<string> const &triggers_in,time_t timeout_in,archive const &a)
{
	rwlock_wrlock lock(access_lock);
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
	cont.data=a.get();
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

};
