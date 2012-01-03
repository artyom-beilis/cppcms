///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/defs.h>
#include "test.h"
#include <cppcms/session_storage.h>
#include "session_memory_storage.h"
#ifdef CPPCMS_WIN_NATIVE
#include "session_win32_file_storage.h"
#include <windows.h>
#else
#include "session_posix_file_storage.h"
#include <sys/types.h>
#include <dirent.h>
#endif
#ifndef CPPCMS_NO_TCP_CACHE
#include "tcp_cache_server.h"
#include "session_tcp_storage.h"
#endif
#include <booster/function.h>
#include <booster/backtrace.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>



std::string dir = "./sessions";
std::string bs="0123456789abcdef0123456789abcde";

void do_nothing() {}

void test(booster::shared_ptr<cppcms::sessions::session_storage> storage,cppcms::sessions::session_storage_factory &f)
{
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

int count_files()
{
#ifndef CPPCMS_WIN_NATIVE
	DIR *d=opendir(dir.c_str());
	TEST(d);
	int counter = 0;
	struct dirent *de;
	while((de=readdir(d))!=0) {
		if(strlen(de->d_name)==32)
			counter++;
	}
	closedir(d);
	return counter;
#else
	WIN32_FIND_DATA entry;
	HANDLE d=FindFirstFile((dir+"/*").c_str(),&entry);
	int counter=0;
	if(d==INVALID_HANDLE_VALUE) {
		return 0;
	}
	do {
		if(strlen(entry.cFileName)==32)
			counter++;
	}while(FindNextFile(d,&entry));
	FindClose(d);
	return counter;
#endif
}

void test_files(booster::shared_ptr<cppcms::sessions::session_storage> storage,
		cppcms::sessions::session_storage_factory &f)
{
	test(storage,f);
	TEST(f.requires_gc());
	time_t now=time(0);
	storage->save(bs+"1",now,"test");
	TEST(count_files()==1);
	storage->remove(bs+"1");
	TEST(count_files()==0);
	storage->save(bs+"1",now-1,"test");
	storage->save(bs+"2",now+1,"test2");
	TEST(count_files()==2);
	f.gc_job();
	TEST(count_files()==1);
	std::string tstr;
	time_t ttime;
	TEST(!storage->load(bs+"1",ttime,tstr));
	TEST(storage->load(bs+"2",ttime,tstr));
	TEST(ttime==now+1 && tstr=="test2");
	storage->save(bs+"2",now-1,"test2");
	TEST(count_files()==1);
	f.gc_job();
	TEST(count_files()==0);
}



int main()
{
	try {

		booster::shared_ptr<cppcms::sessions::session_storage> storage;
		std::auto_ptr<cppcms::sessions::session_storage_factory> storage_factory;
		using namespace cppcms::sessions;

		std::cout << "Testing memory storage" << std::endl;
		session_memory_storage_factory mem;
		storage=mem.get();
		test(storage,mem);
		std::cout << "Testing file storage" << std::endl;
		#ifndef CPPCMS_WIN_NATIVE
		std::cout << "Testing single process" << std::endl;
		session_file_storage_factory f1(dir,5,1,false);
		storage=f1.get();
		test_files(storage,f1);
		std::cout << "Testing multiple process" << std::endl;
		session_file_storage_factory f2(dir,5,5,false);	
		storage=f2.get();
		test_files(storage,f2);
		std::cout << "Testing single process over NFS" << std::endl;
		session_file_storage_factory f3(dir,5,1,true);	
		storage=f3.get();
		test_files(storage,f3);
		#else
		session_file_storage_factory f(dir);
		storage=f.get();
		test_files(storage,f);
		#endif
		#ifndef CPPCMS_NO_TCP_CACHE
		std::cout << "Testing network backend" << std::endl;
		
		std::vector<std::string> ips;
		ips.push_back("127.0.0.1");
		std::vector<int> ports;
		ports.push_back(8080);
		tcp_factory f4(ips,ports);
		booster::shared_ptr<cppcms::sessions::session_storage_factory> 
			mem_ptr(new session_memory_storage_factory());
		cppcms::impl::tcp_cache_service service(0,mem_ptr,1,"127.0.0.1",8080);
		storage=f4.get();
		test(storage,f4);
		service.stop();
		mem_ptr.reset();
		#endif
	}
	catch(std::exception const &e) {
		std::cerr <<"Fail: " << e.what() << std::endl;
		std::cerr << booster::trace(e) << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
