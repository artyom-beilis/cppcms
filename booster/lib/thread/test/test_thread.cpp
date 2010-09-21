//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/thread.h>
#include <booster/posix_time.h>
#include "test.h"
#include <iostream>
#include <stdlib.h>

#ifndef BOOSTER_WIN32
#include <unistd.h>
#endif

bool called_ = false;

void caller()
{
	called_=true;
}

int variable = 0;

template<typename MutexType>
struct incrementer {
	MutexType *mutex_;
	void operator()()
	{
		for(int i=0;i<5;i++) {
			{
				booster::unique_lock<MutexType> ul(*mutex_);
				int tmp = variable;
				booster::ptime::millisleep(20);
				variable = tmp + 1;
			}
		}
	}
};

bool tls_ok = true;
bool dtor_called = false;

struct tls_object {
	int counter;
	tls_object() : counter(0) {}
	~tls_object() { dtor_called = true; }
};

struct tls_functor {
	booster::thread_specific_ptr<tls_object> *ptr;
	tls_functor(booster::thread_specific_ptr<tls_object> &p) : ptr(&p) {}
	void operator()() const
	{
		try {
			booster::thread_specific_ptr<tls_object> &p=*ptr;
			if(p.get()==0)
				p.reset(new tls_object());
			for(int i=0;i<10;i++) {
				int v1 = p->counter;
				booster::ptime::millisleep(25);
				p->counter = v1+1;
				TEST(p->counter == i+1);
			}
			TEST(p->counter == 10);
		}
		catch(...) {
			tls_ok=false;
		}
	}
};

struct cond_incrementer {
	int *counter;
	booster::mutex *m;
	booster::condition_variable *c;
	void operator()() const
	{
		booster::unique_lock<booster::mutex> l(*m);
		c->wait(l);
		++*counter;
	}
};


struct rw_executor {
	bool *flag;
	bool read;
	booster::mutex *flags_mutex;
	booster::shared_mutex *mutex;
	void operator()() const
	{
		for(int i=0;i<20;i++) {
			if(read) {
				mutex->shared_lock();
				{
					flags_mutex->lock();
					*flag = true;
					flags_mutex->unlock();
				}
				booster::ptime::millisleep(5);
				{
					flags_mutex->lock();
					*flag = false;
					flags_mutex->unlock();
				}
				mutex->unlock();
			}
			else {
				mutex->unique_lock();
				*flag = true;
				booster::ptime::millisleep(5);
				*flag = false;
				mutex->unlock();
			}
		}
	}
};




int main()
{
	try {
		{
			std::cout << "Test execution" << std::endl;
			TEST(!called_);
			booster::thread t(caller);
			t.join();
			TEST(called_);
		}
		std::cout << "Test recursive mutex" << std::endl;
		{
			booster::recursive_mutex m;
			m.lock();
			m.lock();
			TEST("Double lock works");
			m.unlock();
			m.unlock();
			TEST("Got there");
		}
		{
			variable = 0;
			booster::recursive_mutex m;
			incrementer<booster::recursive_mutex> inc = { &m };
			booster::thread t1(inc);
			booster::thread t2(inc);
			t1.join();
			t2.join();
			TEST(variable == 10);
		}
		std::cout << "Test mutex" << std::endl;
		{
			variable = 0;
			booster::mutex m;
			incrementer<booster::mutex> inc = { &m };
			booster::thread t1(inc);
			booster::thread t2(inc);
			t1.join();
			t2.join();
			TEST(variable == 10);
		}
		std::cout << "Test rw_lock write lock" << std::endl;
		{
			variable = 0;
			booster::shared_mutex m;
			incrementer<booster::shared_mutex> inc = { &m };
			booster::thread t1(inc);
			booster::thread t2(inc);
			t1.join();
			t2.join();
			TEST(variable == 10);
		}
		std::cout << "Test rw_lock shared/write lock" << std::endl;
		{
			booster::mutex fm;
			booster::shared_mutex sm;
			bool flags[3] = {false,false,false};
			bool mread_happened  = false;
			bool write_happened = false;
			bool error_occured = false ;
			rw_executor exec1 = { flags + 0, true, &fm, &sm };
			rw_executor exec2 = { flags + 1, true, &fm, &sm };
			rw_executor exec3 = { flags + 2, true, &fm, &sm };
			booster::thread t1(exec1);
			booster::thread t2(exec2);
			booster::thread t3(exec3);

			for(int i=0;i<100;i++) {
				booster::ptime::millisleep(1);
				{
					booster::unique_lock<booster::mutex> l(fm);
					if(flags[0] && flags[1])
						mread_happened = true;
					if(flags[2])
						write_happened = true;
					if((flags[0] || flags[1]) && flags[2])
						error_occured = true;
				}
			}

			t1.join();
			t2.join();
			t3.join();

			TEST(mread_happened);
			TEST(write_happened);
			TEST(error_occured);
		}
		std::cout << "Test thread specific" << std::endl;
		{
			booster::thread_specific_ptr<tls_object> p;
			tls_functor f1(p);
			tls_functor f2(p);
			tls_functor f3(p);
			booster::thread t1(f1);
			booster::thread t2(f2);
			booster::thread t3(f3);

			t1.join();
			t2.join();
			t3.join();

			TEST(tls_ok);
			TEST(dtor_called);
		}
		std::cout << "Thest conditional variable notify one" << std::endl;
		{
			int counter = 0;
			booster::mutex m;
			booster::condition_variable c;
			cond_incrementer inc = { &counter, &m , &c };
			booster::thread t1(inc);
			booster::thread t2(inc);
			booster::thread t3(inc);
			booster::ptime::millisleep(100);
			TEST(counter == 0);
			c.notify_one();
			booster::ptime::millisleep(100);
			TEST(counter == 1);
			c.notify_one();
			booster::ptime::millisleep(100);
			TEST(counter == 2);
			c.notify_one();
			booster::ptime::millisleep(100);
			TEST(counter == 3);
			t1.join();
			t2.join();
			t2.join();
		}
		std::cout << "Thest conditional variable notify all" << std::endl;
		{
			int counter[3] = { 0, 0 , 0 };
			booster::mutex m;
			booster::condition_variable c;
			cond_incrementer inc = { counter, &m , &c };
			booster::thread t1(inc);
			inc.counter++;
			booster::thread t2(inc);
			inc.counter++;
			booster::thread t3(inc);
			booster::ptime::millisleep(100);
			TEST(counter[0]==0 && counter[1]==0 && counter[2]==0);
			c.notify_all();
			booster::ptime::millisleep(100);
			TEST(counter[0]==1 && counter[1]==1 && counter[2]==1);
			t1.join();
			t2.join();
			t2.join();
		}
	}
	catch(std::exception const &e)
	{
		std::cerr << "Fail:" <<e.what();
		return EXIT_FAILURE;
	}
	std::cout << "Ok" << std::endl;
	return EXIT_SUCCESS;
	
}
