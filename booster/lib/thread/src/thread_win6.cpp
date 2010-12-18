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
#include <errno.h>
#include <string.h>

//
// This file is for Windows Vista and above with much better
// API so no pthreads library required.
//

namespace booster {

	struct thread::data {
		HANDLE h;
		function<void()> cb;
	};

	extern "C" void *booster_thread_func(void *p)
	{
		thread::data *d=reinterpret_cast<thread::data *>(p);
		try {
			d->cb();
		}
		catch(std::exception const &/*e*/) {
			/// TODO
		}
		catch(...) {
			/// TODO
		}
		_endthreadex(0);
		return 0;
	}
	unsigned WINAPI booster_real_thread_func(void *p) { booster_thread_func(p); return 0; }

	thread::thread(function<void()> const &cb) :
		d(new thread::data)
	{
		d->cb=cb;
		uintptr_t p=_beginthreadex(0,0,booster_real_thread_func,d.get(),0,0);
		if(p==0)
			throw system::system_error(system::error_code(errno,system::system_category));
		d->h=(HANDLE)(p);
	}
	thread::~thread()
	{
		CloseHandle(d->h);
	}
	void thread::join()
	{
		WaitForSingleObject(d->h,INFINITE);
	}
	unsigned thread::hardware_concurrency()
	{
		SYSTEM_INFO info=SYSTEM_INFO();
		GetSystemInfo(&info);
		return info.dwNumberOfProcessors;
	}

	struct mutex::data {
		/// There is no - non - recursive mutex, so we can't do it without this
		CRITICAL_SECTION m;
	};

	mutex::mutex() : d(new data)
	{
		InitializeCriticalSection(&d->m);
	}
	mutex::~mutex()
	{
	}
	void mutex::lock() { 
		EnterCriticalSection(&d->m);
	}
	void mutex::unlock() { 
		LeaveCriticalSection(&d->m);
	}

	struct recursive_mutex::data { CRITICAL_SECTION m; };

	recursive_mutex::recursive_mutex() : d(new data)
	{
		InitializeCriticalSection(&d->m);
	}
	recursive_mutex::~recursive_mutex()
	{
		DeleteCriticalSection(&d->m);
	}
	void recursive_mutex::lock() { EnterCriticalSection(&d->m);}
	void recursive_mutex::unlock() { LeaveCriticalSection(&d->m); }

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

	struct condition_variable::data { CONDITION_VARIABLE c; };

	condition_variable::condition_variable() : d(new data)
	{
		InitializeConditionVariable(&d->c);
	}
	condition_variable::~condition_variable()
	{
	}
	void condition_variable::notify_one()
	{
		WakeConditionVariable(&d->c);
	}
	void condition_variable::notify_all()
	{
		WakeAllConditionVariable(&d->c);
	}
	void condition_variable::wait(unique_lock<mutex> &m)
	{
		SleepConditionVariableCS(&d->c,&(m.mutex()->d->m),INFINITE);
	}

	namespace details {
	struct thread_specific_impl::data {
		std::auto_ptr<object> base;
		DWORD key;
	};

	extern "C" {
		void WINAPI booster_details_thread_specific_impl_deleter(void *pobject)
		{
			if(pobject == 0)
				return;
			delete reinterpret_cast<thread_specific_impl::object *>(pobject);
			
		}
	}

	thread_specific_impl::thread_specific_impl(object *bptr) :
		d(new data())
	{
		d->base.reset(bptr);
		d->key = FlsAlloc(booster_details_thread_specific_impl_deleter);
		
		if(d->key == FLS_OUT_OF_INDEXES)
			throw system::system_error(system::error_code(GetLastError(),system::system_category));
	}

	thread_specific_impl::~thread_specific_impl()
	{
		FlsFree(d->key);
	}
	
	thread_specific_impl::object *thread_specific_impl::get_object() const
	{
		void *pobj = FlsGetValue(d->key);
		if(!pobj) {
			pobj = reinterpret_cast<void *>(d->base->clone());
			if(!FlsSetValue(d->key,pobj)) {
				int err=GetLastError();
				delete reinterpret_cast<object *>(pobj);
				throw system::system_error(system::error_code(err,system::system_category));
			}
		}
		return reinterpret_cast<object *>(pobj);
	}

	} // details

} // booster


