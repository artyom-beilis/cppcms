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
#include <cppcms/defs.h>
#include "test.h"
#include <cppcms/session_storage.h>
#include "session_memory_storage.h"
#include "session_sqlite_storage.h"
#ifdef CPPCMS_WIN_NATIVE
#include "session_win32_file_storage.h"
#include <windows.h>
#else
#include "session_posix_file_storage.h"
#include <sys/types.h>
#include <dirent.h>
#endif
#include <booster/function.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>


std::string dir = "./sessions";
std::string bs="0123456789abcdef0123456789abcde";

void do_nothing() {}

void test(booster::shared_ptr<cppcms::sessions::session_storage> storage,booster::function<void()> callback=do_nothing)
{
	time_t now=time(0)+3;
	callback();
	storage->save(bs+"1",now,"");
	std::string out="xx";
	time_t tout;
	callback();
	TEST(storage->load(bs+"1",tout,out));
	TEST(out.empty());
	TEST(tout==now);
	callback();
	storage->remove(bs+"1");
	callback();
	TEST(!storage->load(bs+"1",tout,out));
	callback();
	storage->save(bs+"1",now-4,"hello world");
	callback();
	TEST(!storage->load(bs+"1",tout,out));
	callback();
	storage->save(bs+"1",now,"hello world");
	callback();
	TEST(storage->load(bs+"1",tout,out));
	callback();
	TEST(out=="hello world");
	storage->save(bs+"2",now,"x");
	callback();
	storage->remove(bs+"2");
	callback();
	TEST(storage->load(bs+"1",tout,out));
	TEST(out=="hello world");
	callback();
	storage->remove(bs+"1");
	callback();
	storage->remove(bs+"2");
	callback();
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
	test(storage);
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


struct do_gc {
	cppcms::sessions::session_storage_factory *f;
	int n;
	void operator()() const
	{
		for(int i=0;i<n;i++) {
			f->gc_job();
		}
	}
};

int main()
{
	try {
		booster::shared_ptr<cppcms::sessions::session_storage> storage;
		std::auto_ptr<cppcms::sessions::session_storage_factory> storage_factory;
		using namespace cppcms::sessions;

		std::cout << "Testing memory storage" << std::endl;
		session_memory_storage_factory mem;
		storage=mem.get();
		test(storage);
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

		std::cout << "Testing sqlite storage" << std::endl;
		remove("test.db");
		
		try {
			storage_factory.reset(cppcms::sessions::sqlite_session::factory("test.db"));
		}
		catch(std::exception const &e) {
			std::string msg = e.what();
			if(	msg.find("Failed to load library")!=std::string::npos 
				|| msg.find("3.7 and above required")!=std::string::npos)
			{
				std::cout << "Seems that sqlite3 storage not supported" << std::endl;
			}
			else
				throw;
		}
		if(storage_factory.get()) {
			storage=storage_factory->get();
			for(int i=0;i<3;i++) {
				std::cout << "- GC " << i << std::endl;
				do_gc gc = { storage_factory.get(), i};
				test(storage,gc);
			}
		}
	}
	catch(std::exception const &e) {
		std::cerr <<"Fail" << e.what() << std::endl;
		return 1;
	}
	return 0;
}
