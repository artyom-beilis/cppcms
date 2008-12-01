#include "cache_interface.h"
#include "worker_thread.h"
#include "global_config.h"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <sstream>
#include <iostream>
#include "manager.h"

namespace cppcms {
using namespace std;

namespace {
	const time_t infty=
		(sizeof(time_t)==4 ? 0x7FFFFFFF: 0x7FFFFFFFFFFFFFFFULL ) - 
			3600*24;
	time_t deadtime(int sec)
	{
		if(sec<0) 
			return infty;
		else {
			time_t tmp;
			time(&tmp);
			if(tmp+sec<tmp) {
				throw cppcms_error("Year 2038 problem?");
			}
			return tmp+sec;
		}
	}
}


void deflate(string const &text,ostream &stream,long level,long length)
{
	using namespace boost::iostreams;
	gzip_params params;
	if(level!=-1){
		params.level=level;
	}

	filtering_ostream zstream;

	if(length!=-1){
		zstream.push(gzip_compressor(params,length));
	}
	else {
		zstream.push(gzip_compressor(params));
	}

	zstream.push(stream);
	zstream<<text;
}

string deflate(string const &text,long level,long length)
{
	ostringstream sstream;
	deflate(text,sstream,level,length);
	return sstream.str();
}


bool cache_iface::fetch_page(string const &key)
{
	if(!cms->caching_module) return false;
	string tmp;
	if(cms->caching_module->fetch_page(key,tmp,cms->gzip)) {
		cms->cout<<tmp;
		cms->gzip_done=true;
		return true;
	}
	return false;
}

void cache_iface::store_page(string const &key,int timeout)
{
	if(!cms->caching_module) return;
	archive a;
	int level=cms->app.config.get<int>("gzip.level",-1);
	int length=cms->app.config.get<int>("gzip.buffer",-1);
	string tmp=cms->out_buf.str();

	string compr=deflate(tmp,level,length);
	a<<tmp<<compr;
	if(cms->gzip){
		cms->out_buf.str("");
		cms->cout<<compr;
		cms->gzip_done=true;
	}
	cms->caching_module->store(key,triggers,deadtime(timeout),a);
}

void cache_iface::add_trigger(string const &t)
{
	if(!cms->caching_module) return;
	triggers.insert(t);
}

void cache_iface::rise(string const &t)
{
	if(!cms->caching_module) return;
	cms->caching_module->rise(t);
}

bool cache_iface::fetch_data(string const &key,serializable &data)
{
	if(!cms->caching_module) return false;
	archive a;
	set<string> new_trig;
	if(cms->caching_module->fetch(key,a,new_trig)) {
		data.load(a);
		triggers.insert(new_trig.begin(),new_trig.end());
		return true;
	}
	return false;
}

void cache_iface::store_data(string const &key,serializable const &data,
			set<string> const &triggers,
			int timeout)
{
	if(!cms->caching_module) return;
	archive a;
	data.save(a);
	this->triggers.insert(triggers.begin(),triggers.end());
	cms->caching_module->store(key,triggers,deadtime(timeout),a);
}

bool cache_iface::fetch_frame(string const &key,string &result)
{
	if(!cms->caching_module) return false;
	archive a;
	set<string> new_trig;
	if(cms->caching_module->fetch(key,a,new_trig)) {
		a>>result;
		triggers.insert(new_trig.begin(),new_trig.end());
		return true;
	}
	return false;
}

void cache_iface::store_frame(string const &key,string const &data,
			set<string> const &triggers,
			int timeout)
{
	if(!cms->caching_module) return;
	archive a;
	a<<data;

	this->triggers.insert(triggers.begin(),triggers.end());
	cms->caching_module->store(key,triggers,deadtime(timeout),a);
}

void cache_iface::clear()
{
	if(cms->caching_module)
		cms->caching_module->clear();
}

bool cache_iface::stats(unsigned &k,unsigned &t)
{
	if(!cms->caching_module)
		return false;
	cms->caching_module->stats(k,t);
	return true;
}

} // End of namespace cppcms
