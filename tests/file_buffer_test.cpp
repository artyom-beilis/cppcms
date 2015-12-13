///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
//#define DEBUG_FILE_BUFFER
#include "http_file_buffer.h"
#include "test.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <iomanip>

char cfpos(int c)
{
	return '0' + c % 10;
}

void test_read(std::istream &is,int &oldpos,int size)
{
	//static_cast<cppcms::http::impl::file_buffer *>(is.rdbuf())->status("Before");
	for(int i=oldpos;i<size;i++) {
		int r = is.get();
		if(r!=cfpos(i))
			printf("pos=%d %d [%c] %c\n",i,r,r,cfpos(i));
		TEST(r==cfpos(i));
	}
	TEST(is.get() == EOF);
	is.clear();
	oldpos = size;
	int limit = 100;
	if(limit > size * 2)
		limit = size * 2;
	for(int i=0;i<limit;i++) {
		int pos = rand() % size;
		int chunk = rand() % size + 1;
		if(chunk+pos  > size) {
			chunk = size - pos;
		}
		switch(i%4) {
		case 0: is.seekg(pos); break;
		case 1: is.seekg(pos,std::ios_base::beg); break;
		case 2: is.seekg(pos - size,std::ios_base::end); break;
		case 3: is.seekg(pos - oldpos,std::ios_base::cur); break;
		}
		TEST(int(is.tellg()) == pos);
		for(int j=0;j<chunk;j++) {
			TEST(is.get() == cfpos(pos + j));
		}
		oldpos = pos + chunk;
		TEST(int(is.tellg()) == oldpos);
		if(oldpos == size) {
			TEST(is.get()==-1);
			is.clear();
			TEST(int(is.tellg()) == oldpos);
		}

		if(rand() % 5==0 && oldpos > 0) {
			int back = rand() % oldpos + 1;
			for(int i=0;i<back;i++) {
				TEST(is.unget());
			}
			oldpos -= back;
		}
		TEST(int(is.tellg()) == oldpos);
	}
}


void test(int const size,int msize)
{
	std::cout << "Testing for file size = " << std::setw(10) << size << " in memory size " << std::setw(8) << msize << std::endl;
	int limit = 100;
	if(limit > size * 2)
		limit = size * 2;
	for(int i=0;i<limit;i++) {
		cppcms::http::impl::file_buffer fb(msize);
		fb.temp_dir(".");
		try {
			std::istream in(&fb);
			std::ostream out(&fb);
			int rpos = 0;
			for(int csize = 0;csize < size;) {
				int chunk = rand() % size + 1;
				if(chunk + csize > size)
					chunk = size - csize;
				for(int i=0;i<chunk;i++) {
					out << cfpos(csize + i);
					TEST(out);
				}
				csize += chunk;
				test_read(in,rpos,csize);
			}
			out<<std::flush;
			if(fb.size() != size)
				printf("%d %d\n",int(fb.size()),size);
			TEST(fb.size() == size);
			TEST(int(out.tellp())==size);
			TEST(fb.in_memory() == (size <= msize));
		}
		catch(...) {
			fb.close();
			if(!fb.name().empty())
				booster::nowide::remove(fb.name().c_str());
			throw;
		}
		fb.close();
		TEST(fb.name().empty() == fb.in_memory());
		if(!fb.in_memory()) {
			fflush(stdout);
			FILE *f=booster::nowide::fopen(fb.name().c_str(),"rb");
			TEST(f);
			for(int i=0;i<size;i++) {
				TEST(fgetc(f)==cfpos(i));
			}
			TEST(fgetc(f)==EOF);
			fclose(f);
			booster::nowide::remove(fb.name().c_str());
		}
	}
}

int main()
{
	try {
		for(int size = 1; size <= 10000; size*= 10) {
			int inmem[] = { 0, 16, 1024, 20000, -1 };
			for(int m=0;inmem[m] != -1;m++) {
				test(size,inmem[m]);
			}
		}
	}
	catch(std::exception const &e) {
		std::cerr << "FAIL: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
