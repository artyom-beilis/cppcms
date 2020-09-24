//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
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
#include <booster/auto_ptr_inc.h>
#include <vector>
#include <stack>

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

	typedef function<void()> thread_function_type;

	struct thread::data {
		pthread_t p;
		bool released;
		data() : released(false) {}
	};

	extern "C" void *booster_thread_func(void *p)
	{
		std::unique_ptr<thread_function_type> caller(reinterpret_cast<thread_function_type *>(p));
		try {
			thread_function_type &func = *caller;
			func();
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
		thread_function_type *ptr = new thread_function_type(cb);
		if( ::pthread_create(&d->p,0,booster_thread_func,reinterpret_cast<void *>(ptr)) == 0 ) {
			// passed pointer - ownership lost
			ptr = 0;
			return;
		}
		else {
			// failed to create - delete the object
			int err = errno;
			delete ptr;
			ptr = 0;
			throw system::system_error(	err,
							system::system_category,
							"booster::thread: failed to create a thread");
		}
	}
	thread::~thread()
	{
		detach();
	}
	void thread::join()
	{
		if(!d->released) {
			pthread_join(d->p,0);
			d->released = true;
		}
	}
	void thread::detach()
	{
		if(!d->released) {
			pthread_detach(d->p);
			d->released = true;
		}
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

	#if !defined(__APPLE__) && !defined(__NetBSD__)
	//
	// This is normal implementation
	//
	// Same as shared mutex under "Most platforms"
	//
	struct recursive_shared_mutex::data { pthread_rwlock_t m; };
	recursive_shared_mutex::recursive_shared_mutex() : d(new data)
	{
		pthread_rwlock_init(&d->m,0);
	}
	recursive_shared_mutex::~recursive_shared_mutex()
	{
		pthread_rwlock_destroy(&d->m);
	}
	void recursive_shared_mutex::shared_lock() { pthread_rwlock_rdlock(&d->m); }
	void recursive_shared_mutex::unique_lock() { pthread_rwlock_wrlock(&d->m); }
	void recursive_shared_mutex::unlock() { pthread_rwlock_unlock(&d->m); }


	#else // Darwin has broken recursive RW Lock
	
	struct recursive_shared_mutex::data {
		thread_specific_ptr<int> k;
		pthread_rwlock_t m;
	};

	namespace {
		int &specific_key(thread_specific_ptr<int> &k)
		{
			int *counter = k.get();
			if(!counter) {
				counter = new int(0);
				k.reset(counter);
			}
			return *counter;
		}
	}

	recursive_shared_mutex::recursive_shared_mutex() : d(new data)
	{
		pthread_rwlock_init(&d->m,0);
	}
	recursive_shared_mutex::~recursive_shared_mutex()
	{
		pthread_rwlock_destroy(&d->m);
	}
	void recursive_shared_mutex::shared_lock()
	{
		int &counter = specific_key(d->k);
		if(counter++ == 0)
			pthread_rwlock_rdlock(&d->m);
	}
	void recursive_shared_mutex::unique_lock() {
		pthread_rwlock_wrlock(&d->m); 
	}
	void recursive_shared_mutex::unlock() { 
		int &counter = specific_key(d->k);
		if(counter > 1) {
			counter --;
			return;
		}
		counter = 0;
		pthread_rwlock_unlock(&d->m);
	}

	#endif

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


#if !defined(__NetBSD__) && !defined(__CYGWIN__)
	//
	// Standard implementation pthread_key_t per object
	//
	namespace details {
		extern "C" {
			static void booster_pthread_key_destroyer(void *p)
			{
				if(!p)
					return;
				delete static_cast<tls_object*>(p);
			}
		}
		
		class pthread_key : public key {
		public:
			pthread_key(void (*d)(void *)) : key(d)
			{
				if(pthread_key_create(&key_,booster_pthread_key_destroyer) != 0) {
					throw system::system_error(errno,system::system_category,
								   "Failed to create thread specific key");
				}
			}
			virtual ~pthread_key()
			{
				pthread_key_delete(key_);
			}
			tls_object *get_object()
			{
				void *p=pthread_getspecific(key_);
				if(p)
					return static_cast<tls_object*>(p);
				tls_object *res = new tls_object(intrusive_ptr<key>(this));
				pthread_setspecific(key_,static_cast<void*>(res));
				return res;
			}
		private:
			pthread_key_t key_;
		};

		intrusive_ptr<key> make_key(void (*dtor)(void *))
		{
			return new pthread_key(dtor);
		}
	} // details
#else // workaround
	//
	// There is a workaround for two major cases:
	//	
	// NetBSD: Workaround of failure to allocate and release keys or if there only few pthread_keys avalible, under NetBSD keys are not being released for some reason
	// 
	// Cygwin: deadlock in destructor - call of pthread_key_delete from the destructor given in pthread key itself
	//
	namespace details {
		typedef std::vector<tls_object *> tls_vector;
		extern "C" {
			static void booster_init_mgr_instance();
			static void booster_detail_keys_deleter(void *ptr)
			{
				if(!ptr)
					return;
				tls_vector *v=static_cast<tls_vector *>(ptr);
				if(!v)
					return;
				for(size_t i=0;i<v->size();i++)
					if((*v)[i])
						delete (*v)[i];
				delete v;
			}
		}

		class keys_manager {
		public:
			typedef booster::unique_lock<booster::mutex> guard_type;
			static keys_manager &instance()
			{
				static pthread_once_t once = PTHREAD_ONCE_INIT;
				pthread_once(&once,booster_init_mgr_instance);
				return *instance_ptr;
			}
			keys_manager() : key_max_(0)
			{
				if(pthread_key_create(&key_,booster_detail_keys_deleter)!=0) {
					throw system::system_error(errno,system::system_category,
						   "Failed to create thread specific key");
				}
			}
			int allocate_key()
			{
				int key;
				guard_type g(lock_);
				if(reuse_pool_.empty()) {
					key = key_max_++;
				}
				else {
					key = reuse_pool_.top();
					reuse_pool_.pop();
				}
				return key;
			}
			void release_key(int key)
			{
				try {
					guard_type g(lock_);
					reuse_pool_.push(key);
				}
				catch(...) {}
			}
			
			tls_vector &get_tls_vector()
			{
				void *p=pthread_getspecific(key_);
				if(!p) {
					tls_vector *v = new tls_vector();
					pthread_setspecific(key_,v);
					return *v;
				}
				return *static_cast<tls_vector *>(p);

			}
			static keys_manager *instance_ptr;
		private:
			booster::mutex lock_;
			int key_max_;
			std::stack<int> reuse_pool_;
			pthread_key_t key_;
			
		};

		keys_manager *keys_manager::instance_ptr;

		extern "C" {
			static void booster_init_mgr_instance()
			{
				static keys_manager mgr;
				keys_manager::instance_ptr = &mgr;
			}
		}

		class unlimited_key : public key {
		public:
			unlimited_key(void (*d)(void *)) : key(d)
			{
				id_ = keys_manager::instance().allocate_key();
			}
			virtual ~unlimited_key()
			{
				keys_manager::instance().release_key(id_);
			}
			tls_object *get_object()
			{
				std::vector<tls_object *> &v=keys_manager::instance().get_tls_vector();
				if(v.size() < size_t(id_) + 1) {
					v.resize(id_+1,0);
				}
				tls_object *p=v[id_];
				if(p)
					return p;
				tls_object *res = new tls_object(intrusive_ptr<key>(this));
				v[id_] = res;
				return res;
	
			}
		private:
			int id_;
		};
		intrusive_ptr<key> make_key(void (*dtor)(void *))
		{
			return new unlimited_key(dtor);
		}
	} // detail
#endif 

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
			throw system::system_error(err,system::system_category,"fork_shared_mutex:failed to create temporary file");
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
		throw system::system_error(err,system::system_category,"fork_shared_mutex: failed to lock");
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
		throw system::system_error(err,system::system_category,"fork_shared_mutex: failed to lock");
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
		throw system::system_error(err,system::system_category,"fork_shared_mutex: failed to lock");
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
		throw system::system_error(err,system::system_category,"fork_shared_mutex: failed to lock");
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


