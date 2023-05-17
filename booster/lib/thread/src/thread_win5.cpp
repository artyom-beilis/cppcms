//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
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


} // booster


