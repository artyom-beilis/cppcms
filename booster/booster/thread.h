//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
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

	///
	/// \brief Mutex object
	///
	class BOOSTER_API mutex : public noncopyable {
	public:
		mutex();
		~mutex();
		///
		/// Lock the mutex. If the same thread tries to lock the mutex it holds, the behavior is undefined
		/// and would likely lead to deadlock
		///
		/// \see unique_lock
		///
		void lock();
		///
		/// Unlock the mutex. If called for unlocked mutex, the behavior is undefined
		///
		/// \see unique_lock
		///
		void unlock();
		friend class condition_variable;
	private:
		struct data;
		hold_ptr<data> d;
	};

	///
	/// \brief Recursive mutex object.
	///
	/// Unlike \ref mutex, when the same thread tries to lock the mutex that is already locked it would
	/// succeed and would require to unlock it same times it was locked.
	///
	/// Useful for handling objects locks when the order of functions calls is not known in advice. For example
	/// \code
	/// void foo()
	/// {
	///    unique_lock<recursive_mutex> guard(this->lock);
	///    bar();
	///    baz();
	/// }
	/// void bar()
	/// {
	///    unique_lock<recursive_mutex> guard(this->lock);
	///    ...
	/// }
	/// \endcode
	///
	/// \see unique_lock
	/// \see shared_lock 
	///
	class BOOSTER_API recursive_mutex : public noncopyable {
	public:
		recursive_mutex();
		~recursive_mutex();
		///
		/// Lock the mutex. \see unique_lock
		///
		void lock();
		///
		/// Unlock the mutex, \see unique_lock
		/// 
		void unlock();
	private:
		struct data;
		hold_ptr<data> d;
	};

	///
	/// \brief Shared mutex or a.k.a. Read-Write Lock
	/// 
	/// This class provides two options of locking unique - nobody but me can use the object
	/// shared anybody with shared lock can use the object.
	///
	class BOOSTER_API shared_mutex : public noncopyable {
	public:
		shared_mutex();
		~shared_mutex();
		///
		/// Same as unique_lock()
		///
		void lock() { unique_lock(); }
		///
		/// Acquire a unique lock on the object. \see booster::unique_lock
		///
		void unique_lock();
		///
		/// Acquire a shared lock on the object.  \see booster::shared_lock
		///
		void shared_lock();
		///
		/// Release the lock
		///
		void unlock();
	private:
		struct data;
		hold_ptr<data> d;
	};

	template<typename Mutex>
	class unique_lock;

	///
	/// \brief This is conditional variable object
	///
	/// For detailed description of the concept read http://en.wikipedia.org/wiki/Monitor_(synchronization)
	///
	class BOOSTER_API condition_variable {
	public:
		condition_variable();
		~condition_variable();
		
		///
		/// Wait for the condition atomically unlocking the mutex referenced by m.
		///
		/// When the condition occurs the lock on the mutex would be acquired again.
		///
		/// Note it is unspecified whether suspicious wakes can occur. It is good idea to check
		/// whether the condition hold after this function returns.
		///
		void wait(unique_lock<mutex> &m);

		///
		/// Notify exactly one waiting process on the condition. If no process waits then the notification
		/// would be ignored
		/// 
		void notify_one();
		///
		/// Notify all waiting process on the condition. If no process waits then the notification
		/// would be ignored
		/// 
		void notify_all();
	private:
		struct data;
		hold_ptr<data> d;
	};

	/// \cond INTERNAL
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

	/// \endcond

	///
	/// \brief Thread specific pointer
	///
	/// Thread specific pointer is NULL for all threads. So on first use it is expected
	/// to be initialized.
	///
	/// The object would be destroyed when the thread exists. If thread_specific_ptr was destroyed
	/// before the thread that uses it exited the object would still exist and would be cleaned up
	/// on thread exit.
	///
	/// Of course an access to thread_specific_ptr object would lead to undefined behavior and would
	/// likely is going to crash your program. 
	///
	/// \note
	///
	/// On Windows platform, when CppCMS and Booster are compiled \b statically (not used as DLL) then
	/// TLS cleanup for the threads that where not created with booster::thread would not be executed.
	///
	/// So when using thread_specific_ptr on Windows platform make sure that you do one of the following:
	///
	/// - Use booster::thread_specific_ptr only from threads created by booster::thread (or main thread)
	/// - Link dynamically with booster library (use it as DLL)
	///
	template<typename T>
	class thread_specific_ptr {
	public:
		///
		/// Create a new thread specific pointer
		///
		thread_specific_ptr() : key_(details::make_key(destructor))
		{
		}

		///
		/// Destroy the thread specific pointer
		///
		~thread_specific_ptr()
		{
		}
		///
		/// Get thread specific value, if not initialized returns NULL
		///
		T *get() const
		{
			return static_cast<T*>(key_->get());
		}
		///
		/// Dereference pointer, if it is NULL the behavior is undefined
		///
		T* operator->() const
		{
			return get();
		}
		///
		/// Dereference pointer, if it is NULL the behavior is undefined
		///
		T& operator*() const
		{
			return *get();
		}
		///
		/// Reset the thread specific pointer and set its value to \a new_val. 
		///
		/// If previous object was not NULL it would be destroyed
		///
		void reset(T *new_val = 0)
		{
			T *p = get();
			if(p)
				destructor(p);
			key_->set(static_cast<void *>(new_val));
		}
		///
		/// Release exiting pointer and set thread specific pointer to 0.
		///
		/// The caller is responsible to destroy the returned object
		///
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


	///
	/// \brief a Unique lock guard.
	///
	/// Acquire the unique lock in the constructor and release it in the destructor
	///
	template<typename Mutex>
	class unique_lock : public noncopyable {
	public:
		/// Acquire the lock
		unique_lock(Mutex &m) : m_(&m)
		{
			m_->lock();
		}
		/// Release the lock
		~unique_lock()
		{
			m_->unlock();
		}
		/// Get the reference to the mutex object
		Mutex *mutex() const
		{
			return m_;
		}
	private:
		Mutex *m_;
	};

	///
	/// \brief a Shared lock guard.
	///
	/// Acquire the shared lock in the constructor and release it in the destructor
	///
	template<typename Mutex>
	class shared_lock : public noncopyable {
	public:
		/// Acquire the lock
		shared_lock(Mutex &m) : m_(&m)
		{
			m_->shared_lock();
		}
		/// Release the lock
		~shared_lock()
		{
			m_->unlock();
		}
		/// Get the reference to the mutex object
		Mutex *mutex() const
		{
			return m_;
		}
	private:
		Mutex *m_;
	};
#ifdef BOOSTER_POSIX
	///
	/// \brief The mutex that can be used by the processes that had forked
	///
	/// \note the lock is automatically released when process dies
	///
	class BOOSTER_API fork_shared_mutex : public noncopyable {
	public:
		///
		/// Create a new mutex - should be done before calling to fork()
		///
		fork_shared_mutex();
		~fork_shared_mutex();

		///
		/// Try to acquire a unique lock on the mutex
		///
		bool try_lock() { return try_unique_lock(); }
		///
		/// Try to acquire a unique lock on the mutex
		///
		bool try_unique_lock();
		///
		/// Try to acquire a shared lock on the mutex
		///
		bool try_shared_lock();

		///
		/// Acquire a unique lock on the mutex
		///
		void lock() { return unique_lock(); }
		///
		/// Acquire a unique lock on the mutex
		///
		void unique_lock();
		///
		/// Acquire a shared lock on the mutex
		///
		void shared_lock();

		///
		/// Release the mutex
		///
		void unlock();
	private:
		struct data;
		hold_ptr<data> d;
	};
#endif

}//booster


#endif
