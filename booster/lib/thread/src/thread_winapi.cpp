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

	namespace winthread {
		typedef std::set<DWORD> keeper;

		DWORD index = TLS_OUT_OF_INDEXES;

		void on_process_start()
		{
			index = TlsAlloc();
			if(index == TLS_OUT_OF_INDEXES) {
				throw booster::runtime_error("Could not allocate thread local index");
			}
		}

		void on_process_end()
		{
			TlsFree(index);
		}

		keeper *get_keeper()
		{
			keeper *r = 0;
			void *ptr=TlsGetValue(index);
			if(!ptr) {
				r = new keeper();
				TlsSetValue(index,static_cast<void*>(r));
			}
			else {
				r = static_cast<keeper *>(ptr);
			}
			return r;
		}

		void on_thread_end()
		{
			void *ptr=TlsGetValue(index);
			if(!ptr)
				return;
			keeper *data = static_cast<keeper *>(ptr);
			for(keeper::iterator p=data->begin();p!=data->end();++p) {
				void *ptr = TlsGetValue(*p);
				if(ptr) {
					delete static_cast<details::tls_object*>(ptr);
					TlsSetValue(*p,0);
				}
			}
			delete data;
			TlsSetValue(index,0);
		}
		
		DWORD tls_alloc()
		{
			DWORD id = TlsAlloc();
			if(id == TLS_OUT_OF_INDEXES) {
				throw booster::runtime_error("Could not allocate thread local key");
			}
			return id;
		}
		void tls_set(DWORD id,void *p)
		{
			get_keeper()->insert(id);
			TlsSetValue(id,p);
		}
		void *tls_get(DWORD id)
		{
			return TlsGetValue(id);
		}
		void tls_free(DWORD id)
		{
			TlsFree(id);
		}
        
		thread_local struct thread_guard {
			thread_guard() {}
			~thread_guard() { on_thread_end(); }
		} tg_instance;


		#ifdef DLL_EXPORT
		extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason,LPVOID)
		{
			switch(reason) {
				case DLL_PROCESS_ATTACH:
					on_process_start();
					break;
				case DLL_PROCESS_DETACH:
					on_thread_end();
					on_process_end();
					break;
				case DLL_THREAD_DETACH:
					on_thread_end();
					break;
			}
			return TRUE;
		}
		#else
		struct win_thread_initializer {
			win_thread_initializer() {
				on_process_start();
			}
			~win_thread_initializer()
			{
				on_thread_end();
				on_process_end();
			}
		} win_thread_initializer_instance;
		#endif
	} // winthread 


	struct recursive_shared_mutex::data {
		mutex lock;
		condition_variable can_lock;

		int read_lock;
		int write_lock;
		int pending_lock;
		
		static const unsigned hash_size = 2048;

		unsigned short recursive_locks[hash_size];

		unsigned static id() 
		{
			return GetCurrentThreadId() % hash_size;
		}

	};
	recursive_shared_mutex::recursive_shared_mutex() : d(new data)
	{
		d->read_lock = 0;
		d->write_lock = 0;
		d->pending_lock = 0;
		memset(&d->recursive_locks,0,sizeof(d->recursive_locks));
	}
	recursive_shared_mutex::~recursive_shared_mutex()
	{
	}
	void recursive_shared_mutex::shared_lock() 
	{ 
		unsigned id = data::id();
		booster::unique_lock<mutex> g(d->lock);
		for(;;) {
			if(d->write_lock == 0 && (d->pending_lock == 0 || d->recursive_locks[id]>0)) {
				d->read_lock++;
				d->recursive_locks[id]++;
				break;
			}
			d->can_lock.wait(g);
		}

	}
	void recursive_shared_mutex::unique_lock() 
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
	void recursive_shared_mutex::unlock() 
	{
		unsigned id = data::id();
		booster::unique_lock<mutex> g(d->lock);
		if(d->write_lock) {
			d->write_lock = 0;
			d->pending_lock = 0;
			d->can_lock.notify_all();
		}
		else if(d->read_lock) {
			d->read_lock--;
			d->recursive_locks[id]--;
			if(d->read_lock == 0)
				d->can_lock.notify_all();
		}
	}

	
	namespace details {
		class wintls_key : public key {
		public:
			wintls_key(void (*d)(void *)) : key(d)
			{
				key_ = winthread::tls_alloc();
			}
			virtual ~wintls_key()
			{
				winthread::tls_free(key_);
			}
			tls_object *get_object()
			{
				void *p=winthread::tls_get(key_);
				if(p)
					return static_cast<tls_object*>(p);
				tls_object *res = new tls_object(intrusive_ptr<key>(this));
				winthread::tls_set(key_,static_cast<void*>(res));
				return res;
			}
		private:
			DWORD key_;
		};

		intrusive_ptr<key> make_key(void (*dtor)(void *))
		{
			return new wintls_key(dtor);
		}
	} // details


} // booster


