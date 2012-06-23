///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include "test.h"
#include "cache_storage.h"
#include "base_cache.h"
#include <booster/intrusive_ptr.h>
#include <booster/posix_time.h>
#include <cppcms/config.h>
#include <iostream>
#include <memory>
#include <time.h>
#include <iomanip>
#include <stdlib.h>
#include <cppcms/urandom.h>


extern "C" void do_store(	cppcms::impl::base_cache &c,
				std::string const &key,
				std::string const &value,
				std::set<std::string> const &tr)
{
	c.store(key,value,tr,time(0)+1000);
}

unsigned long long rand_val()
{
	cppcms::urandom_device rd;
	unsigned long long v;
	rd.generate(&v,sizeof(v));
	return v;
}

void cache_tester(booster::intrusive_ptr<cppcms::impl::base_cache> cache)
{
	std::string page;
	page.reserve(16384);
	while(page.size() < 16384) {
		page+="x";
	}
	unsigned long long rnd[10][10];
	for(int i=0;i<10;i++)
		for(int j=0;j<10;j++)
			rnd[i][j]=rand_val();
	int limit = 10000;
	double store = 0;
	double store2 = 0;
	int store_count=0;
	double rise = 0,rise2=0;
	int rise_count=0;
	for(int i=0;i<limit;i++) {
		std::string key;
		std::set<std::string> triggers;
		for(int j=0;j<10;j++) {
			std::ostringstream ss;
			ss << "trigger_" << j <<"_" << i % 10;
			//ss << "t" << rnd[j][i % 10];
			triggers.insert(ss.str());
		}
		std::ostringstream ss;
		ss << "key_" << i;
		//ss << rand_val();
		key=ss.str();
		{
			booster::ptime start = booster::ptime::now();
			do_store(*cache,key,page,triggers);
			//cache->store(key,page,triggers,time(0)+100);
			booster::ptime end = booster::ptime::now();
			double m = booster::ptime::to_number(end-start);
			store +=m;
			store2 += m*m;
			store_count ++;
		}
		if(i % 1000 == 999) {
			std::ostringstream ss;
			ss << "trigger_0_" << ((i + 324) % 10000 / 1000);
			//ss << "t" << rnd[0][((i + 324) % 10000 / 1000)];
			std::string t=ss.str();
			{
				booster::ptime start = booster::ptime::now();
				cache->rise(t);
				booster::ptime end = booster::ptime::now();
				double m = booster::ptime::to_number(end-start);
				rise +=m;
				rise2 += m*m;
				rise_count ++;
			}
		}
	}

	double mean_store = store / store_count * 1e6;
	double std_store = sqrt(store2 / store_count - (store / store_count) * ( store / store_count)) * 1e6;
	std::cout <<std::fixed<< "Store " <<std::setprecision(1)<< std::setw(16)<< mean_store << " +/- " <<std::setw(16)<< std_store << std::endl;
	double mean_rise = rise / rise_count * 1e6;
	double std_rise = sqrt(rise2 / rise_count - (rise / rise_count) * ( rise / rise_count)) * 1e6;
	std::cout << "Rise  " <<std::setprecision(1)<<std::setw(16)<< mean_rise << " +/- " <<std::setw(16)<< std_rise << std::endl;

}


int main()
{
	try {
		std::locale::global(std::locale(""));
		std::cout.imbue(std::locale());
		std::cout << "Testing thread cache... "<< std::endl;
		cache_tester(cppcms::impl::thread_cache_factory(10000));
		#if !defined(CPPCMS_WIN32) && !defined(CPPCMS_NO_PREFOK_CACHE)
		std::cout << "Testing process cache... " << std::endl;
		cache_tester(cppcms::impl::process_cache_factory(512*1024*1024,10000));
		#endif
		

	}
	catch(std::exception const &e) {
		std::cerr << "\nFail " << e.what() << std::endl;
		return 1;
	}
	return 0;

}
