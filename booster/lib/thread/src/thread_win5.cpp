//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <process.h>
#include <booster/thread.h>
#include <booster/system_error.h>
#include <booster/refcounted.h>
#include <booster/intrusive_ptr.h>
#include <set>
#include <errno.h>
#include <string.h>

#include <iostream>

//
// This file is for Windows 2000 and above, not super
// efficient, but that what we can get from crappy API
//

namespace booster {

	struct mutex::data {
		CRITICAL_SECTION m;
	};

	mutex::mutex() : d(new data)
	{
		InitializeCriticalSection(&d->m);
	}
	mutex::~mutex()
	{
		DeleteCriticalSection(&d->m);
	}
	void mutex::lock() 
	{
		EnterCriticalSection(&d->m);
	}
	void mutex::unlock() 
	{ 
		LeaveCriticalSection(&d->m);
	}
	
	struct shared_mutex::data {
		mutex lock;
		condition_variable can_lock;

		int read_lock;
		int write_lock;
		int pending_lock;

	};
	shared_mutex::shared_mutex() : d(new data)
	{
		d->read_lock = 0;
		d->write_lock = 0;
		d->pending_lock = 0;
	}
	shared_mutex::~shared_mutex()
	{
	}
	void shared_mutex::shared_lock() 
	{ 
		booster::unique_lock<mutex> g(d->lock);
		for(;;) {
			if(d->write_lock == 0 && d->pending_lock == 0) {
				d->read_lock++;
				break;
			}
			else
				d->can_lock.wait(g);
		}

	}
	void shared_mutex::unique_lock() 
	{ 
		booster::unique_lock<mutex> g(d->lock);
		for(;;) {
			if(d->write_lock == 0 && d->read_lock==0) {
				d->write_lock = 1;
				d->pending_lock = 0;
				break;
			}
			else {
				if(d->read_lock)
					d->pending_lock = 1;
				d->can_lock.wait(g);
			}
		}
	}
	void shared_mutex::unlock() 
	{
		booster::unique_lock<mutex> g(d->lock);
		if(d->write_lock) {
			d->write_lock = 0;
			d->pending_lock = 0;
			d->can_lock.notify_all();
		}
		else if(d->read_lock) {
			d->read_lock--;
			if(d->read_lock == 0)
				d->can_lock.notify_all();
		}
	}

	namespace details {
		struct event {
			HANDLE h;
			event()
			{
				h=CreateEvent(0,FALSE,FALSE,0);
				if(!h) throw system::system_error(GetLastError(),
								  system::system_category,
								  "conditional_variable:CreateEvent failed");
				next = 0;
			}
			~event()
			{
				CloseHandle(h);
			}
			void wait()
			{
				WaitForSingleObject(h,INFINITE);
			}
			void set()
			{
				SetEvent(h);
			}
			event *next;
		};
	}

	struct condition_variable::data {
		booster::mutex lock;
		details::event *first;
		details::event *last;
	};

	condition_variable::condition_variable() : d(new data)
	{
		d->first = 0;
		d->last = 0;
	}

	condition_variable::~condition_variable()
	{
	}


	void condition_variable::notify_one()
	{
		booster::unique_lock<booster::mutex> g(d->lock);
		if(d->first == 0)
			return;
		details::event *ev = d->first;
		d->first = d->first->next;
		if(d->first == 0)
			d->last = 0;
		ev->next = 0;
		ev->set(); 
		// should be last as it may be
		// no longer valid
	}

	void condition_variable::notify_all()
	{
		booster::unique_lock<booster::mutex> g(d->lock);
		while(d->first) {
			details::event *ev = d->first;
			d->first = d->first->next;
			ev->next = 0;
			ev->set();
			// should be last as it may be
			// no longer valid
		}
		d->last = 0;
	}

	void condition_variable::wait(unique_lock<mutex> &m)
	{
		details::event ev;
		
		{
			booster::unique_lock<booster::mutex> g(d->lock);
			m.mutex()->unlock();
			if(d->first == 0) {
				d->first = d->last = &ev;
			}
			else {
				d->last->next = &ev;
				d->last = &ev;
			}
	
		}
		
		ev.wait();
		
		m.mutex()->lock();

	}

} // booster


