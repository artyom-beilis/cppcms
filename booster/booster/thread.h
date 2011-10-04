//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_UTIL_THREAD_H
#define BOOSTER_UTIL_THREAD_H

#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>
#include <booster/refcounted.h>
#include <booster/intrusive_ptr.h>
#include <booster/function.h>
#include <booster/config.h>

namespace booster {

	extern "C" void *booster_thread_func(void *);
	
	///
	/// \brief the class that allows to start an execution thread
	///
	class BOOSTER_API thread : public noncopyable {
	public:
		///
		/// Run a function \a cb in separate thread
		///
		thread(function<void()> const &cb);
		~thread();

		///
		/// Join existing thread
		///
		void join();

		///
		/// Detach from the thread
		///
		void detach();

		///
		/// Get number of CPUS, returns 0 if the number is unknown
		///
		static unsigned hardware_concurrency();
	private:
		friend void *booster_thread_func(void *);
		struct data;
		hold_ptr<data> d;
	};


	class condition_variable;

	class BOOSTER_API mutex : public noncopyable {
	public:
		mutex();
		~mutex();
		void lock();
		void unlock();
		friend class condition_variable;
	private:
		struct data;
		hold_ptr<data> d;
	};

	class BOOSTER_API recursive_mutex : public noncopyable {
	public:
		recursive_mutex();
		~recursive_mutex();
		void lock();
		void unlock();
	private:
		struct data;
		hold_ptr<data> d;
	};

	class BOOSTER_API shared_mutex : public noncopyable {
	public:
		shared_mutex();
		~shared_mutex();
		void lock() { unique_lock(); }
		void unique_lock();
		void shared_lock();
		void unlock();
	private:
		struct data;
		hold_ptr<data> d;
	};

	template<typename Mutex>
	class unique_lock;

	class BOOSTER_API condition_variable {
	public:
		condition_variable();
		~condition_variable();

		void wait(unique_lock<mutex> &m);
		void notify_one();
		void notify_all();
	private:
		struct data;
		hold_ptr<data> d;
	};

	namespace details {
		struct tls_object;

		class key : public refcounted {
		public:
			key(void (*d)(void *)) : 
				dtor_(d)
			{
			}
			virtual ~key()
			{
			}
			void *get();
			void set(void *);

			void destroy(void *p)
			{
				dtor_(p);
			}
			virtual tls_object *get_object() = 0;
		private:
			void (*dtor_)(void *);
		};

		BOOSTER_API intrusive_ptr<key> make_key(void (*dtor)(void *));

		struct tls_object {
			tls_object(intrusive_ptr<key> p) :
				the_key(p),
				obj(0)
			{
			}
			~tls_object()
			{
				the_key->destroy(obj);
				obj = 0;
			}
			intrusive_ptr<key> the_key;
			void *obj;
		};

		inline void key::set(void *p)
		{
			get_object()->obj = p;
		}
		inline void *key::get()
		{
			return get_object()->obj;
		}

	} // details

	template<typename T>
	class thread_specific_ptr {
	public:
		thread_specific_ptr() : key_(details::make_key(destructor))
		{
		}
		~thread_specific_ptr()
		{
		}
		T *get() const
		{
			return static_cast<T*>(key_->get());
		}
		T* operator->() const
		{
			return get();
		}
		T& operator*() const
		{
			return *get();
		}
		void reset(T *new_val = 0)
		{
			T *p = get();
			if(p)
				destructor(p);
			key_->set(static_cast<void *>(new_val));
		}
		T *release()
		{
			T *p = get();
			key_->set(0);
			return p;
		}
	private:
		static void destructor(void *ptr)
		{
			delete static_cast<T*>(ptr);
		}
		intrusive_ptr<details::key> key_;
	};


	template<typename Mutex>
	class unique_lock : public noncopyable {
	public:
		unique_lock(Mutex &m) : m_(&m)
		{
			m_->lock();
		}
		~unique_lock()
		{
			m_->unlock();
		}
		Mutex *mutex() const
		{
			return m_;
		}
	private:
		Mutex *m_;
	};

	template<typename Mutex>
	class shared_lock : public noncopyable {
	public:
		shared_lock(Mutex &m) : m_(&m)
		{
			m_->shared_lock();
		}
		~shared_lock()
		{
			m_->unlock();
		}
		Mutex *mutex() const
		{
			return m_;
		}
	private:
		Mutex *m_;
	};
#ifdef BOOSTER_POSIX
	class BOOSTER_API fork_shared_mutex : public noncopyable {
	public:
		fork_shared_mutex();
		~fork_shared_mutex();

		bool try_lock() { return try_unique_lock(); }
		bool try_unique_lock();
		bool try_shared_lock();

		void lock() { return unique_lock(); }
		void unique_lock();
		void shared_lock();

		void unlock();
	private:
		struct data;
		hold_ptr<data> d;
	};
#endif

}//booster


#endif
