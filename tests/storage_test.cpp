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
#include "defs.h"
#include "test.h"
#include "session_storage.h"
#include "session_memory_storage.h"
#ifdef CPPCMS_WIN_NATIVE
#include "session_win32_file_storage.h"
#else
#include "session_posix_file_storage.h"
#endif
#include <iostream>
#include <vector>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/filesystem.hpp>
#else // Internal Boost
#   include <cppcms_boost/filesystem.hpp>
    namespace boost = cppcms_boost;
#endif



std::string dir = "./sessions";
std::string bs="0123456789abcdef0123456789abcde";

void test(cppcms::intrusive_ptr<cppcms::sessions::session_storage> storage)
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
}

std::vector<std::string> list_files()
{
	namespace fs = boost::filesystem;
	std::vector<std::string> files;
	fs::directory_iterator end;
	for(fs::directory_iterator p(dir);p!=end;++p) {
		if(!is_directory(p->status())) {
			files.push_back(p->path().filename());
		}
	}
	return files;
}

void test_files(cppcms::intrusive_ptr<cppcms::sessions::session_storage> storage,
		cppcms::sessions::session_storage_factory &f)
{
	test(storage);
	TEST(f.requires_gc());
	time_t now=time(0);
	storage->save(bs+"1",now,"test");
	TEST(list_files().size()==1);
	storage->remove(bs+"1");
	TEST(list_files().size()==0);
	storage->save(bs+"1",now-1,"test");
	storage->save(bs+"2",now+1,"test2");
	TEST(list_files().size()==2);
	f.gc_job();
	TEST(list_files().size()==1);
	std::string tstr;
	time_t ttime;
	TEST(!storage->load(bs+"1",ttime,tstr));
	TEST(storage->load(bs+"2",ttime,tstr));
	TEST(ttime==now+1 && tstr=="test2");
	storage->save(bs+"2",now-1,"test2");
	TEST(list_files().size()==1);
	f.gc_job();
	TEST(list_files().size()==0);
}


int main()
{
	try {
		cppcms::intrusive_ptr<cppcms::sessions::session_storage> storage;
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
		test_files(storage.f);
		#endif
	}
	catch(std::exception const &e) {
		std::cerr <<"Fail" << e.what() << std::endl;
		return 1;
	}
	return 0;
}
