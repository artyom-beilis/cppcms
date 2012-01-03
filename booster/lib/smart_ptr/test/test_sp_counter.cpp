//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "test.h"

#include <booster/smart_ptr/sp_counted_base.h>
#include <iostream>

class counted : public booster::detail::sp_counted_base {
public:
	virtual void dispose() {}
	virtual void *get_deleter(booster::detail::sp_typeinfo const &) { return 0; }
	counted() {
		counter++;
	}
	virtual ~counted() {
		counter--;
	}
	static int counter;
};

int counted::counter;

int main()
{
	try {
		booster::detail::sp_counted_base *sp=new counted();
		TEST(counted::counter==1);
		TEST(sp->use_count()==1);
		sp->add_ref_copy();
		TEST(sp->use_count()==2);
		sp->add_ref_copy();
		TEST(sp->use_count()==3);
		sp->release();
		TEST(sp->use_count()==2);
		sp->weak_add_ref();
		TEST(sp->use_count()==2);
		TEST(sp->add_ref_lock()==true);
		TEST(sp->use_count()==3);
		sp->release();
		sp->release();
		TEST(sp->use_count()==1);
		sp->release();
		TEST(counted::counter==1);
		TEST(sp->use_count()==0);
		TEST(counted::counter==1);
		TEST(sp->add_ref_lock()==false);
		TEST(counted::counter==1);
		sp->weak_release();
		TEST(counted::counter==0);
		sp=new counted();
		TEST(counted::counter==1);
		sp->release();
		TEST(counted::counter==0);
		sp=new counted();
		TEST(counted::counter==1);
		sp->weak_add_ref();
		TEST(counted::counter==1);
		sp->weak_release();
		TEST(counted::counter==1);
		sp->release();
		TEST(counted::counter==0);
		sp=0;
	}
	catch(std::exception const &e) {
		std::cerr << "Fail" << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
