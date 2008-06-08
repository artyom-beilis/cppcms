#include "base_cache.h"
namespace cppcms {

using namespace std;

bool base_cache::fetch_page(string const &key,string &output,bool gzip)
{
	return false;
}
bool base_cache::fetch(string const &key,archive &a,set<string> &tags)
{
	return false;
};

void base_cache::clear()
{
	// Nothing
}
void base_cache::rise(string const &trigger)
{
	// Nothing
}

void base_cache::store(string const &key,set<string> const &triggers,time_t timeout,archive const &a)
{
	// Nothing
}

base_cache::~base_cache()
{
	// Nothing
}

void base_cache::stats(unsigned &keys,unsigned &triggers)
{
	keys=0;
	triggers=0;
}

}

