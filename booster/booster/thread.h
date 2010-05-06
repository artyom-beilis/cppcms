#ifndef BOOSTER_UTIL_THREAD_H
#define BOOSTER_UTIL_THREAD_H

#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>
#include <booster/function.h>
#include <booster/config.h>

#include <memory>

namespace booster {

	extern "C" void *booster_thread_func(void *);

	class BOOSTER_API thread : public noncopyable {
	public:
		thread(function<void()> const &cb);
		~thread();

		void join();
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
		class BOOSTER_API thread_specific_impl : public noncopyable {
		public:
			struct object {
				virtual object *clone() const = 0;
				virtual ~object()
				{
				}
			};
			thread_specific_impl(object *p);
			~thread_specific_impl();
			
			object *get_object() const;
		private:
			struct data;
			hold_ptr<data> d;
		};
	}

	template<typename T>
	class thread_specific_ptr : private details::thread_specific_impl {
		struct real_object;
	public:
		thread_specific_ptr() : details::thread_specific_impl(new real_object())
		{
		}
		~thread_specific_ptr()
		{
		}
		T *get() const
		{
			return pointer()->get();
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
			pointer()->reset(new_val);
		}
		T *release()
		{
			return pointer()->release();
		}
	private:
		std::auto_ptr<T> *pointer() const
		{
			return &static_cast<real_object *>(get_object())->ptr;
		}
		struct real_object : public object {
			std::auto_ptr<T> ptr;
			virtual object *clone() const { return new real_object(); }
		};
	};


	template<typename Mutex>
	class BOOSTER_API unique_lock : public noncopyable {
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
	class BOOSTER_API shared_lock : public noncopyable {
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

}//booster


#endif
