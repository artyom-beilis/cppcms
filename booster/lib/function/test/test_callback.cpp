//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/callback.h>
#include "test.h"
#include <iostream>

bool foov_called;
void foov() { 
	foov_called=true;
}
bool fooi_called;
int fooi()
{
	fooi_called=true;
	return 10;
}
bool fooid_called;

int fooid(double const &v)
{
	TEST(v==10.0);
	fooid_called = true;
	return 0;
}

int fooii(int &x)
{
	return x++;
}

void reset()
{
	foov_called = fooi_called = fooid_called = false;
}

struct call_counter {
	int counter;
	call_counter() : counter(0) {}
	int operator()() { return ++counter; }
};

bool mycall_called = false;

struct mycall : public booster::callable<void()> 
{
	void operator()()
	{
		mycall_called = true;
	}
};

struct myicall : public booster::callable<void(int)> 
{
	void operator()(int /*x*/)
	{
		mycall_called = true;
	}
};



int main()
{
	using booster::callback;
	try {
		reset();
		callback<void()> f=foov;
		f();
		TEST(foov_called);
		f=fooi;
		f();
		TEST(fooi_called);
		callback<int()> fi=fooi;
		TEST(fi()==10);
		TEST(fi);
		callback<void(int)> fvi;
		TEST(!fvi);
		try {
			fvi(10);
			throw std::runtime_error("Not throws!");
		}
		catch(booster::bad_callback_call const &e) {}
		fvi=fooid;
		TEST(fvi);
		fvi(10);
		TEST(fooid_called);
		int x=2;
		TEST(callback<int(int&)>(fooii)(x)==2);
		TEST(x==3);

		callback<int()> fcnt1=call_counter();
		callback<int()> fcnt2=fcnt1;
		TEST(fcnt1()==1);
		TEST(fcnt1()==2);
		TEST(fcnt2()==3);
		TEST(fcnt2()==4);
		
		{
			std::auto_ptr<mycall> mc(new mycall());
			callback<void()> f(mc);
			f();
			TEST(mycall_called); mycall_called=false;
		}
		{
			booster::intrusive_ptr<mycall> mc(new mycall());
			callback<void()> f(mc);
			f();
			TEST(mycall_called); mycall_called=false;
		}
		{
			std::auto_ptr<mycall> mc(new mycall());
			callback<void()> f;
			f=mc;
			f();
			TEST(mycall_called); mycall_called=false;
		}
		{
			booster::intrusive_ptr<mycall> mc(new mycall());
			callback<void()> f;
			f=mc;
			f();
			TEST(mycall_called); mycall_called=false;
		}
		{
			std::auto_ptr<myicall> mc(new myicall());
			callback<void(int)> f(mc);
			f(2);
			TEST(mycall_called); mycall_called=false;
		}
		{
			booster::intrusive_ptr<myicall> mc(new myicall());
			callback<void(int)> f(mc);
			f(2);
			TEST(mycall_called); mycall_called=false;
		}
		{
			std::auto_ptr<myicall> mc(new myicall());
			callback<void(int)> f;
			f=mc;
			f(2);
			TEST(mycall_called); mycall_called=false;
		}
		{
			booster::intrusive_ptr<myicall> mc(new myicall());
			callback<void(int)> f;
			f=mc;
			f(2);
			TEST(mycall_called); mycall_called=false;
		}


	}
	catch(std::exception const &e)
	{
		std::cout << "Fail"<< e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
