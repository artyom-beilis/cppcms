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
#include <map>
#include <errno.h>
#include <string.h>

#include <iostream>

//
// This file is for Windows 2000 and above, not super
// efficient, but that what we can get from crappy API
//

namespace booster {

	namespace winthread {
		struct keeper {
			typedef std::map<void const *,details::thread_specific_impl::object *> objects_type;
			objects_type objects;
			~keeper()
			{
				for(objects_type::iterator p=objects.begin();p!=objects.end();++p) {
					// just in case catch-it-all
					try { delete p->second; }catch(...){}
				}
			}
		};

		DWORD index = TLS_OUT_OF_INDEXES;

		void on_process_start()
		{
			index = TlsAlloc();
			if(index == TLS_OUT_OF_INDEXES) {
				throw system::system_error(system::error_code(GetLastError(),system::system_category));
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
			delete data;
			TlsSetValue(index,0);
		}

		#ifdef DLL_EXPORT
		extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason,LPVOID)
		{
			switch(reason) {
				case DLL_PROCESS_ATTACH:
					on_process_start();
					break;
				case DLL_PROCESS_DETACH:
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
			event(HANDLE tmp)
			{
				if(tmp == 0) {
					h=CreateEvent(0,FALSE,FALSE,0);
					if(!h) throw system::system_error(system::error_code(
								GetLastError(),
								system::system_category));
				}
				else
					h=tmp;
				next = 0;
			}
			HANDLE release_handle()
			{
				HANDLE tmp = h;
				h=0;
				return tmp;
			}
			~event()
			{
				if(h)
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
		booster::mutex cache_lock;
		std::vector<HANDLE> h_cache;
	};

	condition_variable::condition_variable() : d(new data)
	{
		d->first = 0;
		d->last = 0;
		d->h_cache.reserve(64);
	}

	condition_variable::~condition_variable()
	{
		for(size_t i=0;i<d->h_cache.size();i++)
			CloseHandle(d->h_cache[i]);
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
		HANDLE my_handle = 0;
		{
			booster::unique_lock<booster::mutex> g(d->cache_lock);
			if(!d->h_cache.empty()) {
				my_handle = d->h_cache.back();
				d->h_cache.pop_back();
				
			}
		}
		details::event ev(my_handle);
		
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
		
		{
			booster::unique_lock<booster::mutex> g(d->cache_lock);
			d->h_cache.push_back(ev.release_handle());
		}

	}

	namespace details {
	struct thread_specific_impl::data {
		std::auto_ptr<object> base;
	};

	thread_specific_impl::thread_specific_impl(object *bptr) :
		d(new data())
	{
		d->base.reset(bptr);
	}

	thread_specific_impl::~thread_specific_impl()
	{
	}
	
	thread_specific_impl::object *thread_specific_impl::get_object() const
	{
		winthread::keeper *k=winthread::get_keeper();
		winthread::keeper::objects_type::iterator p = k->objects.find(this);
		if(p==k->objects.end()) {
			object *new_ob = d->base->clone();
			k->objects[this]=new_ob;
			return new_ob;
		}
		else {
			return p->second;
		}
	}

	} // details


} // booster


