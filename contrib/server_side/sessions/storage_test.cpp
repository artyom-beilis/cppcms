///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/defs.h>
#include <stdexcept>
#include <sstream>
#include <booster/posix_time.h>
#include <booster/thread.h>
#include <cppcms/session_storage.h>
#include <cppcms/urandom.h>
#include <cppcms/json.h>
#include <booster/function.h>
#include <booster/shared_object.h>
#include <booster/backtrace.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <iomanip>
#include <stdio.h>

#define TEST(X) 								\
	do {									\
		if(X) break;							\
		std::cerr << "Error " << __FILE__ << ":"<<__LINE__ << " "#X << std::endl;		\
		std::ostringstream oss;						\
		oss << "Error " << __FILE__ << ":"<<__LINE__ << " "#X;		\
		throw std::runtime_error(oss.str());				\
	}while(0)	

std::string bs="0123456789abcdef0123456789abcde";

void do_nothing() {}


void test(booster::shared_ptr<cppcms::sessions::session_storage> storage,cppcms::sessions::session_storage_factory &f)
{
	f.gc_job();
	time_t now=time(0)+3;
	storage->save(bs+"1",now,"");
	std::string out="xx";
	time_t tout;
	TEST(storage->load(bs+"1",tout,out));
	TEST(out.empty());
	TEST(tout==now);
	storage->remove(bs+"1");
	TEST(!storage->load(bs+"1",tout,out));
	storage->save(bs+"1",now-4,"hello world");
	TEST(!storage->load(bs+"1",tout,out));
	storage->save(bs+"1",now,"hello world");
	TEST(storage->load(bs+"1",tout,out));
	TEST(out=="hello world");
	storage->save(bs+"2",now,"x");
	storage->remove(bs+"2");
	TEST(storage->load(bs+"1",tout,out));
	TEST(out=="hello world");
	storage->remove(bs+"1");
	storage->remove(bs+"2");
	f.gc_job();
}


std::string get_sid()
{
	cppcms::urandom_device dev;
	unsigned char buf[16];
	char sid[33];
	dev.generate(buf,sizeof(buf));
	static const char digits[17] = "0123456789abcdef";
	for(int i=0;i<16;i++) {
		sid[i*2] = digits[buf[i] >> 4]; 
		sid[i*2 + 1] = digits[buf[i] & 0x0f]; 
	}
	sid[32]=0;
	return sid;
}

std::string message = "this is some long long message";


booster::mutex total_inserted_lock;
int total_inserted;
booster::ptime end_point;

struct thread_runner{
	thread_runner(cppcms::sessions::session_storage_factory &f) : fact_(&f)
	{
		expired_ = time(0) - 5;
	}
	void operator()() const
	{
		try {
			int my_inserted = 0;
			while(booster::ptime::now() < end_point) {
				booster::shared_ptr<cppcms::sessions::session_storage> storage = fact_->get();
				storage->save(get_sid(),expired_,message);
				my_inserted ++;
			}
			{
				booster::unique_lock<booster::mutex> g(total_inserted_lock);
				total_inserted+=my_inserted;
			}
		}catch(std::exception const &e) {
			fprintf(stderr,"Fail %s\n",e.what());
			abort();
		}
		
	}
	cppcms::sessions::session_storage_factory *fact_;
	time_t expired_;
};

struct gc_runner{
	gc_runner(cppcms::sessions::session_storage_factory &f) : fact_(&f)
	{
	}
	void operator()() const
	{
		while(booster::ptime::now() < end_point) {
			booster::ptime::sleep(booster::ptime::seconds(1));
			fact_->gc_job();
		}
	}
	cppcms::sessions::session_storage_factory *fact_;
};


void performance_test(cppcms::sessions::session_storage_factory &f,bool run_gc)
{
	time_t expired = time(0) - 5;
	booster::shared_ptr<cppcms::sessions::session_storage> storage = f.get();
	booster::ptime start,end;

	f.gc_job();
	
	end = booster::ptime::now() + booster::ptime::from_number(1);
	int count = 0;
	for(count = 0;booster::ptime::now() < end ;count ++) {
		f.get()->save(get_sid(),expired,message);
	}
	double insert_single_thread_us = 1.0 / count * 1e6;

	start = booster::ptime::now();
	f.gc_job();
	end = booster::ptime::now();
	double gc_time_us = booster::ptime::to_number(end - start) / count * 1e6;
	
	std::vector<booster::shared_ptr<booster::thread> > threads;
	int threads_no = 10;
	double seconds_to_run = 5;
	end_point = booster::ptime::now() + booster::ptime::from_number(seconds_to_run);
	total_inserted = 0;
	booster::shared_ptr<booster::thread> gc_thread;
	for(int i=0;i<threads_no;i++) {
		booster::shared_ptr<booster::thread> t;
		t.reset(new booster::thread(thread_runner(f)));
		threads.push_back(t);
	}
	if(run_gc)
		gc_thread.reset(new booster::thread(gc_runner(f)));

	for(int i=0;i<threads_no;i++)
		threads[i]->join();
	
	if(gc_thread)
		gc_thread->join();

	double insert_multiple_threads_us =  seconds_to_run / total_inserted * 1e6;
	std::cout << "Benchmarks: "<< std::endl;
	std::cout << std::setprecision(1) << std::fixed;
	std::cout << "  Cleanup message/s:           " << std::setw(10) << 1e6/gc_time_us << std::endl;
	std::cout << "  Inserts / s single thread:   " << std::setw(10) << 1e6/insert_single_thread_us << std::endl;
	std::cout << "  Inserts / s multiple thread: " << std::setw(10) << 1e6/insert_multiple_threads_us << std::endl;
	std::cout << "  Cleanup for message (us):    " << std::setw(10) << gc_time_us << std::endl;
	std::cout << "  Insert single thread (us):   " << std::setw(10) << insert_single_thread_us << std::endl;
	std::cout << "  Insert multiple thread (us): " << std::setw(10) << insert_multiple_threads_us << std::endl;
	
}

int main(int argc,char **argv)
{
	bool failed = false;
	try {
		cppcms::json::value v;
		if(argc==2) {
			std::ifstream f(argv[1]);
			if(!f) {
				throw std::runtime_error("Failed to open input file");
			}
			f >> v;
			if(!f) {
				std::cerr<< "Parsing failed" << std::endl;
				return 1;
			}
		}
		else {
			std::cin >> v;

			if(!std::cin) {
				std::cerr<< "Parsing failed" << std::endl;
				return 1;
			}
		}

		cppcms::json::array &all=v.array();


		for(size_t i=0;i<all.size() && !failed;i++) {
			std::string so = v[i].get<std::string>("so");
			std::string clean = v[i].get("clean","");
			bool run_gc = v[i].get("run_gc",false);
			if(!clean.empty()) {
				system(clean.c_str());
			}
			std::cout << "- Module: " << so << std::endl << v[i].get("test","") << std::endl;
			booster::shared_object obj(so);
			{
				booster::shared_ptr<cppcms::sessions::session_storage> storage;
				std::auto_ptr<cppcms::sessions::session_storage_factory> storage_factory;
				cppcms::sessions::cppcms_session_storage_generator_type gen;
				obj.symbol(gen,"sessions_generator");
				try {
					storage_factory.reset(gen(v[i]));
					storage = storage_factory->get();
					test(storage,*storage_factory);
					std::cout << "-- Complete" << std::endl;
					if(v[i].get("no_performance",false)==false)
						performance_test(*storage_factory,run_gc);
				}
				catch(std::exception const &e) {
					std::cerr << e.what() << std::endl;
					std::cerr << booster::trace(e) << std::endl;
					storage.reset();
					storage_factory.reset();
					obj.close();
					break;
				}
				storage.reset();
				storage_factory.reset();
			}
			obj.close();
		}

	}
	catch(std::exception const &e) {
		std::cerr <<"Fail: " << e.what() << std::endl;
		std::cerr << booster::trace(e) << std::endl;
		return 1;
	}
	if(failed)
		std::cerr << "Fail" << std::endl;
	else
		std::cout << "Ok" << std::endl;
	return 0;
}
