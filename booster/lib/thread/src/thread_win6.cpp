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
#ifndef _WIN32_WINNT 
#define _WIN32_WINNT 0x600
#endif
#include <windows.h>
#include <process.h>
#include <booster/thread.h>
#include <booster/system_error.h>
#include <booster/refcounted.h>
#include <booster/intrusive_ptr.h>
#include <errno.h>
#include <string.h>

//
// This file is for Windows Vista and above with much better
// API so no pthreads library required.
//

namespace booster {
	
	struct shared_mutex::data { SRWLOCK m; bool ex; };
	shared_mutex::shared_mutex() : d(new data)
	{
		d->ex=false;
		InitializeSRWLock(&d->m);
	}
	shared_mutex::~shared_mutex()
	{
	}
	void shared_mutex::shared_lock() { 
		AcquireSRWLockShared(&d->m);
	}
	void shared_mutex::unique_lock() { 
		AcquireSRWLockExclusive(&d->m);
		d->ex=true;
	}
	void shared_mutex::unlock() {
		bool ex = d->ex;
		if(ex) {
			d->ex=false;
			ReleaseSRWLockExclusive(&d->m);
		}
		else 
			ReleaseSRWLockShared(&d->m);
	}

} // booster


