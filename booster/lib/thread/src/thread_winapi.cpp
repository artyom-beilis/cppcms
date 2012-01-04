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


	class win_thread_data : public refcounted {
	public:
		win_thread_data(function<void()> const &cb) :
			handle_(0),
			is_complete_(false),
			callback_(cb)
		{
		}

		void transfer_handle_ownership(HANDLE h)
		{
			{
				unique_lock<mutex> guard(lock_);
				if(!is_complete_) {
					handle_ = h;
					h = 0;
				}
			}

			if(h) {
				CloseHandle(h);
			}
		}
		void run()
		{
			try {
				callback_();
				callback_ = function<void()>();
			}
			catch(...) {}

			winthread::on_thread_end();
			
			unique_lock<mutex> guard(lock_);
			is_complete_ = true;
		}
		~win_thread_data()
		{
			try {
				unique_lock<mutex> guard(lock_);
				if(handle_) {
					CloseHandle(handle_);
					handle_ = 0;
				}
			}
			catch(...) {}
		}
	private:
		mutex lock_;
		HANDLE handle_;
		bool is_complete_;
		function<void()> callback_;
	};
	
	struct thread::data {
		intrusive_ptr<win_thread_data> shared;
		HANDLE h;
	};


	extern "C" void *booster_thread_func(void *p)
	{
		// Do not add reference count as it was added upon sucesseful thread creation
		intrusive_ptr<win_thread_data> d(reinterpret_cast<win_thread_data *>(p),false);
		try {
			d->run();
		}
		catch(...) {
		}
		_endthreadex(0);
		
		return 0;
	}
	unsigned WINAPI booster_real_thread_func(void *p) { booster_thread_func(p); return 0; }

	thread::thread(function<void()> const &cb) :
		d(new thread::data)
	{
		d->shared=new win_thread_data(cb);

		uintptr_t p=_beginthreadex(0,0,booster_real_thread_func,d->shared.get(),0,0);
		if(p!=0) {
			// we want to transfer ownership to the thread explicitly
			intrusive_ptr_add_ref(d->shared.get());
		}
		else {
			throw system::system_error(system::error_code(errno,system::system_category));
		}
		
		d->h=(HANDLE)(p);
	}
	void thread::detach()
	{
		if(d->h && d->shared) {
			d->shared->transfer_handle_ownership(d->h);
			d->h = 0;
			d->shared = 0;
		}
	}
	thread::~thread()
	{
		try {
			detach();
		}
		catch(...){}
	}

	void thread::join()
	{
		if(d->h) {
			WaitForSingleObject(d->h,INFINITE);
			d->h = 0;
			d->shared = 0;
		}
	}

	unsigned thread::hardware_concurrency()
	{
		SYSTEM_INFO info=SYSTEM_INFO();
		GetSystemInfo(&info);
		return info.dwNumberOfProcessors;
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


	struct shared_mutex::data {
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
	shared_mutex::shared_mutex() : d(new data)
	{
		d->read_lock = 0;
		d->write_lock = 0;
		d->pending_lock = 0;
		memset(&d->recursive_locks,0,sizeof(d->recursive_locks));
	}
	shared_mutex::~shared_mutex()
	{
	}
	void shared_mutex::shared_lock() 
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


