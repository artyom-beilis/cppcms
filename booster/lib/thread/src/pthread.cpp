//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <pthread.h>
#include <booster/thread.h>
#include <booster/system_error.h>
#include <errno.h>
#include <string.h>

#ifdef BOOSTER_POSIX
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#endif

#ifdef BOOSTER_WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#else
#  include <unistd.h>
#endif

namespace booster {


	struct thread::data {
		pthread_t p;
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
		return 0;
	}

	thread::thread(function<void()> const &cb) :
		d(new thread::data)
	{
		d->cb=cb;
		::pthread_create(&d->p,0,booster_thread_func,d.get());
	}
	thread::~thread()
	{
	}
	void thread::join()
	{
		pthread_join(d->p,0);
	}

	unsigned thread::hardware_concurrency()
	{
		#ifdef BOOSTER_WIN32
			SYSTEM_INFO info=SYSTEM_INFO();
			GetSystemInfo(&info);
			return info.dwNumberOfProcessors;
		#else
			#ifdef _SC_NPROCESSORS_ONLN
				long procs = sysconf(_SC_NPROCESSORS_ONLN);
				if(procs < 0)
					return 0;
				return procs;
			#else
				return 0;
			#endif
		#endif
	}

	struct mutex::data { pthread_mutex_t m; };

	mutex::mutex() : d(new data)
	{
		pthread_mutex_init(&d->m,0);
	}
	mutex::~mutex()
	{
		pthread_mutex_destroy(&d->m);
	}
	void mutex::lock() { pthread_mutex_lock(&d->m); }
	void mutex::unlock() { pthread_mutex_unlock(&d->m); }

	struct recursive_mutex::data { pthread_mutex_t m; };

	recursive_mutex::recursive_mutex() : d(new data)
	{
		pthread_mutexattr_t a;
		pthread_mutexattr_init(&a);
		pthread_mutexattr_settype(&a,PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&d->m,&a);
	}
	recursive_mutex::~recursive_mutex()
	{
		pthread_mutex_destroy(&d->m);
	}
	void recursive_mutex::lock() { pthread_mutex_lock(&d->m); }
	void recursive_mutex::unlock() { pthread_mutex_unlock(&d->m); }

	struct shared_mutex::data { pthread_rwlock_t m; };
	shared_mutex::shared_mutex() : d(new data)
	{
		pthread_rwlock_init(&d->m,0);
	}
	shared_mutex::~shared_mutex()
	{
		pthread_rwlock_destroy(&d->m);
	}
	void shared_mutex::shared_lock() { pthread_rwlock_rdlock(&d->m); }
	void shared_mutex::unique_lock() { pthread_rwlock_wrlock(&d->m); }
	void shared_mutex::unlock() { pthread_rwlock_unlock(&d->m); }

	struct condition_variable::data { pthread_cond_t c; };

	condition_variable::condition_variable() : d(new data)
	{
		pthread_cond_init(&d->c,0);
	}
	condition_variable::~condition_variable()
	{
		pthread_cond_destroy(&d->c);
	}
	void condition_variable::notify_one()
	{
		pthread_cond_signal(&d->c);
	}
	void condition_variable::notify_all()
	{
		pthread_cond_broadcast(&d->c);
	}
	void condition_variable::wait(unique_lock<mutex> &m)
	{
		pthread_cond_wait(&d->c,&(m.mutex()->d->m));
	}

	namespace details {
	struct thread_specific_impl::data {
		std::auto_ptr<object> base;
		pthread_key_t key;
	};

	extern "C" {
		void booster_details_thread_specific_impl_deleter(void *pobject)
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
		if(pthread_key_create(&d->key,booster_details_thread_specific_impl_deleter) < 0)
			throw system::system_error(system::error_code(errno,system::system_category));
	}

	thread_specific_impl::~thread_specific_impl()
	{
		pthread_key_delete(d->key);
	}
	
	thread_specific_impl::object *thread_specific_impl::get_object() const
	{
		void *pobj = pthread_getspecific(d->key);
		if(!pobj) {
			pobj = reinterpret_cast<void *>(d->base->clone());
			if(pthread_setspecific(d->key,pobj) < 0) {
				int err=errno;
				delete reinterpret_cast<object *>(pobj);
				throw system::system_error(system::error_code(err,system::system_category));
			}
		}
		return reinterpret_cast<object *>(pobj);
	}

	} // details

#ifdef BOOSTER_POSIX

	struct fork_shared_mutex::data {
		pthread_rwlock_t lock;
		FILE *lock_file;
	};


	fork_shared_mutex::fork_shared_mutex() : d(new data)
	{
		pthread_rwlock_init(&d->lock,0);
		d->lock_file = tmpfile();
		if(!d->lock_file) {
			int err=errno;
			pthread_rwlock_destroy(&d->lock);
			throw system::system_error(system::error_code(err,system::system_category));
		}
	}
	fork_shared_mutex::~fork_shared_mutex()
	{
		// only threads see the lock - it is safe
		fclose(d->lock_file);
		pthread_rwlock_destroy(&d->lock);
	}
	
	bool fork_shared_mutex::try_unique_lock()
	{
		if(pthread_rwlock_trywrlock(&d->lock)!=0)
			return false;
		struct flock lock;
		memset(&lock,0,sizeof(lock));
		lock.l_type=F_WRLCK;
		lock.l_whence=SEEK_SET;
		int res = 0;
		while((res = ::fcntl(fileno(d->lock_file),F_SETLK,&lock))!=0 && errno==EINTR)
			;
		if(res == 0)
			return true;
		int err = errno;
		pthread_rwlock_unlock(&d->lock);
		if(err == EACCES || err==EAGAIN)
			return false;
		throw system::system_error(system::error_code(err,system::system_category));
	}
	void fork_shared_mutex::unique_lock()
	{
		pthread_rwlock_wrlock(&d->lock);
		struct flock lock;
		memset(&lock,0,sizeof(lock));
		lock.l_type=F_WRLCK;
		lock.l_whence=SEEK_SET;
		int res = 0;
		while((res = ::fcntl(fileno(d->lock_file),F_SETLKW,&lock))!=0 && errno==EINTR)
			;
		if(res == 0)
			return;
		int err = errno;
		pthread_rwlock_unlock(&d->lock);
		throw system::system_error(system::error_code(err,system::system_category));
	}

	bool fork_shared_mutex::try_shared_lock()
	{
		if(pthread_rwlock_tryrdlock(&d->lock)!=0)
			return false;
		struct flock lock;
		memset(&lock,0,sizeof(lock));
		lock.l_type=F_RDLCK;
		lock.l_whence=SEEK_SET;
		int res = 0;
		while((res = ::fcntl(fileno(d->lock_file),F_SETLK,&lock))!=0 && errno==EINTR)
			;
		if(res == 0)
			return true;
		int err = errno;
		pthread_rwlock_unlock(&d->lock);
		if(err == EACCES || err==EAGAIN)
			return false;
		throw system::system_error(system::error_code(err,system::system_category));
	}

	void fork_shared_mutex::shared_lock()
	{
		pthread_rwlock_rdlock(&d->lock);
		struct flock lock;
		memset(&lock,0,sizeof(lock));
		lock.l_type=F_RDLCK;
		lock.l_whence=SEEK_SET;
		int res = 0;
		while((res = ::fcntl(fileno(d->lock_file),F_SETLKW,&lock))!=0 && errno==EINTR)
			;
		if(res == 0)
			return;
		int err = errno;
		pthread_rwlock_unlock(&d->lock);
		throw system::system_error(system::error_code(err,system::system_category));
	}

	void fork_shared_mutex::unlock()
	{
		struct flock lock;
		memset(&lock,0,sizeof(lock));
		lock.l_type=F_UNLCK;
		lock.l_whence=SEEK_SET;
		int res = 0;
		while((res = ::fcntl(fileno(d->lock_file),F_SETLKW,&lock))!=0 && errno==EINTR)
			;
		pthread_rwlock_unlock(&d->lock);
	}
#endif

} // booster


