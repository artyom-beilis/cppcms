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
#include <stdexcept>
#include <sstream>
#include <cppcms/session_storage.h>
#include <cppcms/json.h>
#include <booster/function.h>
#include <booster/shared_object.h>
#include <booster/backtrace.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>


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
	bool failed = false;
	try {
		cppcms::json::value v;
		std::cin >> v;

		if(!std::cin) {
			std::cerr<< "Parsing failed" << std::endl;
			return 1;
		}

		cppcms::json::array &all=v.array();


		for(size_t i=0;i<all.size() && !failed;i++) {
			std::string so = v[i].get<std::string>("so");
			std::string clean = v[i].get("clean","");
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
					std::cout << "-- Without gc" << std::endl;
					test(storage);
					std::cout << "-- With gc" << std::endl;
					do_gc gc = { storage_factory.get() };
					test(storage,gc);
					std::cout << "-- Complete" << std::endl;
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
