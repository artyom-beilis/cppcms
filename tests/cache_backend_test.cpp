#include "test.h"
#include "cache_storage.h"
#include "base_cache.h"
#include "intrusive_ptr.h"
#include "test.h"
#include <iostream>

#ifdef CPPCMS_WIN_NATIVE
#include <windows.h>
void ssleep(int x) { Sleep(x*1000); }
#else
#include <unistd.h>
void ssleep(int x) { sleep(x); }
#endif




void test_cache(cppcms::intrusive_ptr<cppcms::impl::base_cache> cache)
{
	std::string tmp;
	std::set<std::string> tags;
	TEST(cache->fetch("foo",tmp,0)==false);
	time_t t_and_1=time(0)+1,ttmp=0;
	cache->store("foo","bar",tags,t_and_1);
	TEST(cache->fetch("foo",tmp,&tags)==true);
	TEST(tmp=="bar");
	TEST(tags.size()==1);
	TEST(tags.find("foo")!=tags.end());
	TEST(cache->fetch("foo",0,0,&ttmp)==true);
	TEST(ttmp==t_and_1);
	ssleep(2);
	TEST(cache->fetch("foo",tmp,0)==false);
	tags.clear();
	cache->clear();
	tags.insert("beep");
	cache->store("foo","bar",tags,time(0)+5);
	cache->store("bar","foo",tags,time(0)+5);
	tags.clear();
	cache->store("bee","bzzz",tags,time(0)+5);
	TEST(cache->fetch("foo",tmp,0) && tmp=="bar");
	TEST(cache->fetch("bar",tmp,0) && tmp=="foo");
	TEST(cache->fetch("bee",tmp,0) && tmp=="bzzz");
	unsigned keys=0,triggers=0;
	cache->stats(keys,triggers);
	TEST(keys==3 && triggers==5);
	cache->rise("beep");
	cache->stats(keys,triggers);
	TEST(keys==1 && triggers==1);
	TEST(cache->fetch("foo",tmp,0)==false);
	TEST(cache->fetch("bar",tmp,0)==false);
	TEST(cache->fetch("bee",tmp,0) && tmp=="bzzz");
	cache->clear();
	tags.clear();
	cache->store("fu","fu",tags,time(0)+5);
	for(unsigned i=0;i<1000;i++) {
		std::ostringstream key;
		key << i;
		cache->store(key.str(),key.str(),tags,time(0)+5);
		TEST(cache->fetch("fu",tmp,0) && tmp=="fu");
	}
	TEST(cache->fetch("fu",tmp,0) && tmp=="fu");
	TEST(cache->fetch("1",tmp,0)==false);
	TEST(cache->fetch("999",tmp,0)==true && tmp=="999");
	cache->clear();
	tags.clear();
	cache->store("test","test",tags,time(0)+5);
	uint64_t g1=0,g2=0;
	TEST(cache->fetch("test",0,0,0,&g1));
	cache->store("test","test2",tags,time(0)+5);
	TEST(cache->fetch("test",&tmp,0,0,&g2));
	TEST(tmp=="test2");
	TEST(g1!=0 && g2!=0 && g1!=g2);
}


int main()
{
	try {
		std::cout << "Testing thread cache... "<< std::flush;
		test_cache(cppcms::impl::thread_cache_factory(20));
		std::cout << "Ok" << std::endl;
		#ifndef CPPCMS_WIN32
		std::cout << "Testing process cache... " << std::flush;
		test_cache(cppcms::impl::process_cache_factory(16*1024));
		std::cout << "Ok" << std::endl;
		#endif
	}
	catch(std::exception const &e) {
		std::cerr << "\nFail " << e.what() << std::endl;
		return 1;
	}
	return 0;

}
