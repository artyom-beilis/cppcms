///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#include "test.h"
#include "cache_storage.h"
#include "tcp_cache_server.h"
#include "cache_over_ip.h"
#include "base_cache.h"
#include <booster/intrusive_ptr.h>
#include <cppcms/config.h>
#include "test.h"
#include <iostream>
#include <memory>
#include <time.h>
#ifdef CPPCMS_WIN_NATIVE
#include <windows.h>
void ssleep(int x) { Sleep(x*1000); }
#else
#include <unistd.h>
void ssleep(int x) { sleep(x); }
#endif




void test_cache(booster::intrusive_ptr<cppcms::impl::base_cache> cache,bool test_generators=true)
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
	cache->store("fu","fu",tags,time(0)+25); 
	// 25 - make sure it works on slow platforms like
	// ARM emulator
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
	if(test_generators) {
		cache->store("test","test",tags,time(0)+5);
		cppcms::uint64_t my_gen=2;
		cache->store("test2","x",tags,time(0)+5,&my_gen);
		cppcms::uint64_t g1=0,g2=0,g3=0;
		TEST(cache->fetch("test",0,0,0,&g1));
		cache->store("test","test2",tags,time(0)+5);
		TEST(cache->fetch("test",&tmp,0,0,&g2));
		TEST(tmp=="test2");
		TEST(g1!=0 && g2!=0 && g1!=g2);
		TEST(cache->fetch("test2",0,0,0,&g3));
		TEST(g3==my_gen);
		cache->clear();
	}
}

void test_two_clients(booster::intrusive_ptr<cppcms::impl::base_cache> c1,booster::intrusive_ptr<cppcms::impl::base_cache> c2)
{
	std::string tmp;
	std::set<std::string> tags;
	c1->store("foo","test",tags,time(0)+1);
	TEST(c2->fetch("foo",tmp,0));
	TEST(tmp=="test");
	c2->store("foo","test2",tags,time(0)+1);
	TEST(c1->fetch("foo",tmp,0) && tmp=="test2");
	c2->rise("foo");
	TEST(c1->fetch("foo",tmp,0)==false);
	c2->store("foo","test3",tags,time(0)+1);
	TEST(c1->fetch("foo",tmp,0) && tmp=="test3");
	c2->clear();
	TEST(c1->fetch("foo",tmp,0)==false);
}


int main()
{
	try {
		std::cout << "Testing thread cache... "<< std::flush;
		test_cache(cppcms::impl::thread_cache_factory(20));
		std::cout << "Ok" << std::endl;
		#if !defined(CPPCMS_WIN32) && !defined(CPPCMS_NO_PREFOK_CACHE)
		std::cout << "Testing process cache... " << std::flush;
		test_cache(cppcms::impl::process_cache_factory(16*1024));
		std::cout << "Ok" << std::endl;
		#endif
		
		std::auto_ptr<cppcms::impl::tcp_cache_service> srv1,srv2;
		{
			try {
				std::cout << "Testing cache over ip, single server... "<<std::flush;
				std::vector<std::string> ips;
				std::vector<int> ports;
				ips.push_back("127.0.0.1");
				ports.push_back(6001);
				srv1.reset(new cppcms::impl::tcp_cache_service(cppcms::impl::thread_cache_factory(20),1,"127.0.0.1",6001));

				test_cache(cppcms::impl::tcp_cache_factory(ips,ports,0),false);
				std::cout << "Ok" << std::endl;
				std::cout << "Testing cache over ip, single server with L1 cache... "<<std::flush;
				test_cache(cppcms::impl::tcp_cache_factory(ips,ports,cppcms::impl::thread_cache_factory(5)),false);
				std::cout << "Ok" << std::endl;
				srv2.reset(new cppcms::impl::tcp_cache_service(cppcms::impl::thread_cache_factory(20),1,"127.0.0.1",6002));
				ips.push_back("127.0.0.1");
				ports.push_back(6002);
				std::cout << "Testing cache over ip, multiple server... "<<std::flush;
				test_cache(cppcms::impl::tcp_cache_factory(ips,ports,0),false);
				std::cout << "Ok" << std::endl;
				std::cout << "Testing cache over ip, multiple server with L1 cache... "<<std::flush;
				test_cache(cppcms::impl::tcp_cache_factory(ips,ports,cppcms::impl::thread_cache_factory(5)),false);
				std::cout << "Ok" <<std::endl;
				ips.resize(1);
				ports.resize(1);
				std::cout << "Testing two clients... "<<std::flush;
				test_two_clients(
					cppcms::impl::tcp_cache_factory(ips,ports,cppcms::impl::thread_cache_factory(5)),
					cppcms::impl::tcp_cache_factory(ips,ports,cppcms::impl::thread_cache_factory(5)));
				std::cout << "Ok" <<std::endl;
				std::cout << "Testing two clients with L1 cache... "<<std::flush;
				test_two_clients(
					cppcms::impl::tcp_cache_factory(ips,ports,0),
					cppcms::impl::tcp_cache_factory(ips,ports,0));
				std::cout << "Ok" <<std::endl;
					
			}
			catch(...) {
				if(srv1.get()) {
					srv1->stop();
					srv1.reset();
				}
				if(srv2.get()) {
					srv2->stop();
					srv2.reset();
				}
				throw;
			}
			srv1->stop();	
			srv2->stop();
			
		}

	}
	catch(std::exception const &e) {
		std::cerr << "\nFail " << e.what() << std::endl;
		return 1;
	}
	return 0;

}
