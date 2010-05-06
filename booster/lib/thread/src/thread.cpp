#define BOOSTER_SOURCE
#include <pthread.h>
#include <booster/thread.h>
#include <booster/system_error.h>
#include <errno.h>

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
		catch(std::exception const &e) {
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

} // booster


