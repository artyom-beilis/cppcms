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

	/*
	 * This currently does not work...
	 * Windows got Deadlock during destruction of the TLS key
	 
	namespace details {
		extern "C" {
			static WINAPI void booster_fls_key_destroyer(void *p)
			{
				if(!p)
					return;
				delete static_cast<tls_object*>(p);
			}
		}
		
		class fls_key : public key {
		public:
			fls_key(void (*d)(void *)) : key(d)
			{
				key_ = FlsAlloc(booster_fls_key_destroyer);
				if(key_ == FLS_OUT_OF_INDEXES) {
					throw  booster::runtime_error("Could not allocate Thread specific key");
				}
			}
			virtual ~fls_key()
			{
				FlsFree(key_);
			}
			tls_object *get_object()
			{
				void *p=FlsGetValue(key_);
				if(p)
					return static_cast<tls_object*>(p);
				tls_object *res = new tls_object(intrusive_ptr<key>(this));
				FlsSetValue(key_,static_cast<void*>(res));
				return res;
			}
		private:
			DWORD key_;
		};

		intrusive_ptr<key> make_key(void (*dtor)(void *))
		{
			return new fls_key(dtor);
		}
	} // details

	*/
} // booster


