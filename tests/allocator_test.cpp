///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define TEST_ALLOCATOR
#include "shmem_allocator.h"
#include "test.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>

static const size_t objects = 100;

void *memory[objects];
size_t sizes[objects];
char vals[objects];

void check_all()
{
	for(size_t pos = 0;pos < objects;pos++) {
		if(!memory[pos])
			continue;
		char c=vals[pos];
		size_t size = sizes[pos]; 
		char *s = static_cast<char *>(memory[pos]);
		for(size_t j=0;j<size;j++) {
			TEST(s[j]==c);
		}
	}
}

int main()
{
	try{
		int fail = 0;
		cppcms::impl::shmem_control ct(1024*1024*10ULL);
		int limit = 1000;

		for(int i=0;i<limit;i++) {
			size_t pos = rand() % objects;
			size_t size = 1 << (rand() % 22);
			if(memory[pos]) {
				ct.free(memory[pos]);
				memory[pos] = 0;
				check_all();
			}
			if((memory[pos]=ct.malloc(size))==0) {
				check_all();
				fail++;
			}
			else {
				sizes[pos] = size;
				char c=rand();
				vals[pos] = c;
				char *s = static_cast<char *>(memory[pos]);
				for(size_t j=0;j<size;j++)
					s[j] = c;
				check_all();
			}
		}
		for(size_t i=0;i<objects;i++) {
			if(memory[i]) {
				ct.free(memory[i]);
				memory[i]=0;
				check_all();
			}
		}

		ct.test_free();
		std::cout << double(fail) / limit * 100 << std::endl;
	}
	catch(std::exception const &e) {
		std::cerr << "Fail " << e.what() << booster::trace(e) << std::endl;
		return 1;
	}
	std::cout << "Ok\n";
}



