///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define TEST_ALLOCATOR
//#define DEBUG_ALLOCATOR
#include "test.h"
#include "buddy_allocator.h"
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <iomanip>

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
	size_t memory_size = 1024*1024*10ULL;
	void *ptr = malloc(memory_size);
	cppcms::impl::buddy_allocator *buddy=new(ptr) cppcms::impl::buddy_allocator(memory_size);
	try{
		int fail = 0;

		int limit = 1000;

		buddy->test_free();
		for(int i=0;i<limit;i++) {
			if(i*100 % limit==0)
				std::cout << std::setw(5) << i*100.0 / limit << "%"<< "\b\b\b\b\b\b" << std::flush;
			size_t pos = rand() % objects;
			size_t size = (1 << (rand() % 22));
			size += rand() % size;
			if(memory[pos]) {
				buddy->free(memory[pos]);
				memory[pos] = 0;
				check_all();
			}
			if((memory[pos]=buddy->malloc(size))==0) {
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
			buddy->test_consistent(memory,objects);
		}
		for(size_t i=0;i<objects;i++) {
			if(memory[i]) {
				buddy->free(memory[i]);
				memory[i]=0;
				check_all();
			}
			buddy->test_consistent(memory,objects);
		}

		buddy->test_free();
		std::cout << "\n malloc fail to all " << double(fail) / limit * 100 << std::endl;
		for(int dir=0;dir<=1;dir++) {
			for(size_t i=1;i<memory_size;i*=2) {
				std::cout << "Object size " << i << std::endl;
				std::vector<void *> ptrs;
				void *tmp=0;
				size_t exp =  memory_size / i;
				while((tmp=buddy->malloc(i))!=0) {
					ptrs.push_back(tmp);
					if(ptrs.size() * 100 % exp==0)
						buddy->test_consistent(&ptrs[0],ptrs.size());
				}
				if(ptrs.empty())
					break;
				buddy->test_consistent(&ptrs[0],ptrs.size());
				if(dir == 0) {
					for(size_t i=0;i<ptrs.size();i++) {
						buddy->free(ptrs[i]);
						ptrs[i]=0;
						if(i*100 % ptrs.size() == 0)
							buddy->test_consistent(&ptrs[0]+i,ptrs.size()-i);
					}
				}
				else {
					for(int i=ptrs.size()-1;i>=0;i--) {
						buddy->free(ptrs[i]);
						ptrs[i]=0;
						if(i*100 % ptrs.size() == 0)
							buddy->test_consistent(&ptrs[0],i);
					}
				}
				buddy->test_free();
			}
		}

	}
	catch(std::exception const &e) {
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	buddy->~buddy_allocator();
	free(ptr);
	std::cout << "Ok\n";
}



