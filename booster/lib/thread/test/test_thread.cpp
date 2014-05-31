//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
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
booster::mutex dtor_called_mutex;
int dtor_called = 0;

struct counted_functor {
	counted_functor() { add(1); } 
	counted_functor(counted_functor const &)  { add(1); }
	counted_functor const &operator=(counted_functor const &) { return *this; }
	~counted_functor() { add(-1); }

	void operator()() const
	{
		booster::ptime::millisleep(200);
	}

	static void add(int v)
	{
		booster::unique_lock<booster::mutex> g(lock);
		objects+= v;
	}

	static booster::mutex lock;
	static int objects;

};

booster::mutex counted_functor::lock;
int counted_functor::objects;

struct tls_object {
	int counter;
	tls_object() : counter(0) {}
	~tls_object() {
		booster::unique_lock<booster::mutex> guard(dtor_called_mutex);
		dtor_called++; 
	}
};

struct tls_functor2 {
	booster::thread_specific_ptr<tls_object> *ptr;
	tls_functor2(booster::thread_specific_ptr<tls_object> &p) : ptr(&p) {}
	void operator()() const
	{
		ptr->reset(new tls_object());
		booster::ptime::millisleep(200);
	}
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


template<typename ShM>
struct rw_executor {
	bool *flag;
	bool read;
	booster::mutex *flags_mutex;
	ShM *mutex;
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


struct rw_shared_thread {
	booster::recursive_shared_mutex *lp;
	bool *done;
	void operator()() const
	{
		lp->shared_lock();
		booster::ptime::millisleep(500);
		booster::ptime start = booster::ptime::now();
		lp->shared_lock();
		booster::ptime end = booster::ptime::now();
		booster::ptime::millisleep(200);
		lp->unlock();
		lp->unlock();
		TEST(end-start < booster::ptime::milliseconds(100));
		*done = true;
	}
};

struct rw_unique_thread {
	booster::recursive_shared_mutex *lp;
	bool *done;
	void operator()() const
	{
		booster::ptime::millisleep(250);
		booster::ptime start = booster::ptime::now();
		lp->unique_lock();
		booster::ptime end = booster::ptime::now();
		lp->unlock();
		TEST((end - start) > booster::ptime::milliseconds(100));
		*done = true;
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
		{
			std::cout << "Functor destruction" << std::endl;
			TEST(counted_functor::objects==0);
			{
				counted_functor f;
				booster::thread t(f);
				TEST(counted_functor::objects>=2);
				t.join();
				TEST(counted_functor::objects==1);
			}
			TEST(counted_functor::objects==0);
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
			TEST(dtor_called == 3);
		}
		std::cout << "Non synchronous thread specific" << std::endl;
		{
			dtor_called = 0;
			booster::thread_specific_ptr<tls_object> *p = new booster::thread_specific_ptr<tls_object>();
			
			tls_functor2 f1(*p);
			tls_functor2 f2(*p);
			tls_functor2 f3(*p);

			booster::thread t1(f1);
			booster::thread t2(f2);
			booster::thread t3(f3);

			booster::ptime::millisleep(300);
			delete p;
			
			t1.join();
			t2.join();
			t3.join();

			TEST(dtor_called == 3);
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
			t3.join();
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
			t3.join();
		}
		std::cout << "Test shared_mutex write lock" << std::endl;
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
		std::cout << "Test recursive_shared_mutex write lock" << std::endl;
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
		std::cout << "Test shared_mutex shared/write lock" << std::endl;
		{
			booster::mutex fm;
			booster::shared_mutex sm;
			bool flags[3] = {false,false,false};
			bool mread_happened  = false;
			bool write_happened = false;
			bool error_occured = false ;
			rw_executor<booster::shared_mutex> exec1 = { flags + 0, true, &fm, &sm };
			rw_executor<booster::shared_mutex> exec2 = { flags + 1, true, &fm, &sm };
			rw_executor<booster::shared_mutex> exec3 = { flags + 2, true, &fm, &sm };
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
		std::cout << "Test recursive_shared_mutex shared/write lock" << std::endl;
		{
			booster::mutex fm;
			booster::recursive_shared_mutex sm;
			bool flags[3] = {false,false,false};
			bool mread_happened  = false;
			bool write_happened = false;
			bool error_occured = false ;
			rw_executor<booster::recursive_shared_mutex> exec1 = { flags + 0, true, &fm, &sm };
			rw_executor<booster::recursive_shared_mutex> exec2 = { flags + 1, true, &fm, &sm };
			rw_executor<booster::recursive_shared_mutex> exec3 = { flags + 2, true, &fm, &sm };
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
		std::cout << "Test recursive_shared_mutex recursive shared lock" << std::endl;
		{
			booster::recursive_shared_mutex l;
			bool read  = false;
			bool write = false;
			rw_shared_thread t1c = { &l, &read };
			rw_unique_thread t2c = { &l, &write };
			booster::thread t1(t1c);
			booster::thread t2(t2c);
			t1.join();
			t2.join();
			TEST(read);
			TEST(write);
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
