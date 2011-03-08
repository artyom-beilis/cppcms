//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/shared_ptr.h>
#include <booster/weak_ptr.h>
#include <booster/enable_shared_from_this.h>

#include "test.h"
#include <iostream>

//
//  This is not full regression test, it is rather the test to make sure that everithing is compiled
// 
//  The shared_ptr code is taken from boost_1_41... Hopefully works ;)
//

class foo {
public:
	virtual ~foo(){}
};
class bar : public foo {
public:
	virtual ~bar(){}
};

class bee : public booster::enable_shared_from_this<bee> {
public:
	bee()
	{
	}
	booster::shared_ptr<bee> self()
	{ 
		return shared_from_this();
	}
};


struct counted {
	counted() 
	{
		counter++;
	}
	~counted() 
	{
		counter--;
	}
	counted(counted const &)
	{
		counter++;
	}
	counted const &operator=(counted const &)
	{
		return *this;
	}
	static int counter;
};

int counted::counter;




int main()
{
	try {
		booster::shared_ptr<int> p(new int(10));
		booster::shared_ptr<int> p2=p;
		TEST(*p2==10);
		TEST(p2.use_count() == 2);
		*p2=20;
		TEST(*p==20);
		booster::weak_ptr<int> wp=p;
		TEST(p.use_count() == 2);
		booster::shared_ptr<int> p3=wp.lock();
		TEST(p3);
		TEST(p3.use_count() == 3);
		TEST(*p2==20);
		TEST(!wp.expired());
		TEST(p);
		TEST(!!p);
		p.reset();
		p2.reset();
		TEST(!p);
		TEST(p3.use_count() == 1);
		p3.reset();
		TEST(!p3);
		TEST(wp.expired());
		booster::shared_ptr<int> p4=wp.lock();
		TEST(!p4);

		try {
			booster::shared_ptr<int> p4(wp);
			std::runtime_error("Should not get there");
		}
		catch(booster::bad_weak_ptr const &e){}

		booster::shared_ptr<foo> f(new foo);
		booster::shared_ptr<bar> b(new bar);
		f=b;
		TEST(booster::dynamic_pointer_cast<bar>(f)==b);
		TEST(booster::static_pointer_cast<bar>(f)==b);
		f.reset(new foo);
		TEST(booster::dynamic_pointer_cast<bar>(f)==booster::shared_ptr<bar>());

		booster::shared_ptr<bee> bee1(new bee);
		booster::shared_ptr<bee> bee2=bee1->self();
		TEST(bee1==bee2);

		booster::shared_ptr<counted> cnt;
		TEST(counted::counter==0);
		cnt.reset(new counted);
		TEST(counted::counter==1);
		cnt.reset();
		TEST(counted::counter==0);
		cnt.reset(new counted);
		TEST(counted::counter==1);
		booster::shared_ptr<counted> cnt2(cnt);
		TEST(counted::counter==1);
		cnt.reset(new counted);
		TEST(counted::counter==2);
		cnt=cnt;
		cnt2=cnt2;
		TEST(counted::counter==2);
		cnt=cnt2;
		TEST(counted::counter==1);
		cnt2=cnt;
		TEST(counted::counter==1);
		booster::weak_ptr<counted> wpcnt(cnt2);
		booster::shared_ptr<counted> cnt3=wpcnt.lock();
		TEST(cnt3);
		TEST(counted::counter==1);
		cnt3.reset();
		TEST(counted::counter==1);
		cnt2.reset();
		TEST(counted::counter==1);
		cnt.reset();
		TEST(counted::counter==0);
		TEST(!wpcnt.lock());

	}
	catch(std::exception const &e) {
		std::cerr << "Fail:" << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;	
}
