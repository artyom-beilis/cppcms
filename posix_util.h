#ifndef CPPCMS_POSIX_UTIL_H
#define CPPCMS_POSIX_UTIL_H
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "cppcms_error.h"
#include "noncopyable.h"

namespace cppcms {
namespace impl {

	inline void *mmap_anonymous(size_t size)
	{
		#if defined(MAP_ANONYMOUS)
		void *p=::mmap(0,size,PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
		int err=errno;
		#elif defined(MAP_ANON)
		void *p=::mmap(0,size,PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
		int err=errno;
		#else
		int fd=::open("/dev/null",O_RDWR);
		if(fd < 0) 
			throw cppcms_error(errno,"Failed to open /dev/null");
		void *p=::mmap(0,size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
		int err=errno;
		::close(fd);
		#endif
		if(p==MAP_FAILED)
			throw cppcms_error(err,"Failed to create shared memory");
		return p;
	}

	inline void create_mutex(pthread_mutex_t *m,bool pshared = false)
	{
		if(!pshared) {
			pthread_mutex_init(m,0);
		}
		else {
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			try {
				int res;
				res = pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
				if(res==0)
					res = pthread_mutex_init(m,&attr);
				if(res < 0)
					throw cppcms_error(errno,"Failed to create process shared mutex");
				pthread_mutexattr_destroy(&attr);
			}
			catch(...) {
				pthread_mutexattr_destroy(&attr);
				throw;
			}
		}
	}

	inline void destroy_mutex(pthread_mutex_t *m)
	{
		pthread_mutex_destroy(m);
	}
	
	inline void create_rwlock(pthread_rwlock_t *m,bool pshared=false)
	{
		if(!pshared) {
			pthread_rwlock_init(m,0);
		}
		else {
			pthread_rwlockattr_t attr;
			pthread_rwlockattr_init(&attr);
			try {
				int res;
				res = pthread_rwlockattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
				if(res==0)
					res = pthread_rwlock_init(m,&attr);
				if(res < 0)
					throw cppcms_error(errno,"Failed to create process shared mutex");
				pthread_rwlockattr_destroy(&attr);
			}
			catch(...) {
				pthread_rwlockattr_destroy(&attr);
				throw;
			}
		}
	}

	inline void destroy_rwlock(pthread_rwlock_t *m)
	{
		pthread_rwlock_destroy(m);
	}


	
	inline bool test_pthread_mutex_pshared_impl()
	{
		void *memory=mmap_anonymous(sizeof(pthread_mutex_t));
		bool res=false;
		try {
			create_mutex(reinterpret_cast<pthread_mutex_t *>(memory),true);
			destroy_mutex(reinterpret_cast<pthread_mutex_t *>(memory));
			res=true;
		}
		catch(cppcms_error const &e) {
			res= false;
		}
		::munmap((char*)memory,sizeof(pthread_mutex_t));
		return res;
	}

	inline bool test_pthread_mutex_pshared()
	{
		static bool has = test_pthread_mutex_pshared_impl();
		return has;
	}


	class mutex : public util::noncopyable {
	public:
		class guard;
		mutex(bool pshared = false) :
			plock_(0),
			flock_(0)
		{
			bool use_pthread=test_pthread_mutex_pshared();
			if(use_pthread) {
				plock_ =reinterpret_cast<pthread_mutex_t *>(mmap_anonymous(sizeof(pthread_mutex_t)));
				create_mutex(plock_,pshared);
			}
			else {
				plock_=&normal_;
				create_mutex(plock_);
				if(pshared) {
					flock_=tmpfile();
					if(!flock_) {
						int err=errno;
						destroy_mutex(plock_);
						throw cppcms_error(err,"Failed to create temporary file");
					}
				}
			}
		}

		void lock()
		{
			pthread_mutex_lock(plock_);
			if(flock_) {
				struct flock lock;
				memset(&lock,0,sizeof(lock));
				lock.l_type=F_WRLCK;
				lock.l_whence=SEEK_SET;
				while(::fcntl(fileno(flock_),F_SETLKW,&lock)!=0 && errno==EINTR)
					;
			}
		}

		void unlock()
		{
			if(flock_) {
				struct flock lock;
				memset(&lock,0,sizeof(lock));
				lock.l_type=F_UNLCK;
				lock.l_whence=SEEK_SET;
				while(::fcntl(fileno(flock_),F_SETLKW,&lock)!=0 && errno==EINTR)
					;
			}
			pthread_mutex_unlock(plock_);
		}

		~mutex()
		{
			if(flock_) ::fclose(flock_);
			destroy_mutex(plock_);
			if(plock_ != &normal_)
				::munmap((char*)plock_,sizeof(pthread_mutex_t));
		}
	private:
		pthread_mutex_t *plock_;
		FILE *flock_;
		pthread_mutex_t normal_;
	};

	class mutex::guard : public util::noncopyable {
	public:
		guard(mutex &m) : m_(m) 
		{
			m_.lock();
		}
		~guard()
		{
			m_.unlock();
		}
	private:
		mutex &m_;
	};
	
	class shared_mutex : public util::noncopyable {
	public:
		class shared_guard;
		class unique_guard;
		shared_mutex(bool pshared = false) :
			plock_(0),
			flock_(0)
		{
			bool use_pthread=test_pthread_mutex_pshared();
			if(use_pthread) {
				plock_ =reinterpret_cast<pthread_rwlock_t *>(mmap_anonymous(sizeof(pthread_rwlock_t)));
				create_rwlock(plock_,pshared);
			}
			else {
				plock_=&normal_;
				create_rwlock(plock_);
				if(pshared) {
					flock_=tmpfile();
					if(!flock_) {
						int err=errno;
						destroy_rwlock(plock_);
						throw cppcms_error(err,"Failed to create temporary file");
					}
				}
			}
		}

		void wrlock()
		{
			pthread_rwlock_wrlock(plock_);
			if(flock_) {
				struct flock lock;
				memset(&lock,0,sizeof(lock));
				lock.l_type=F_WRLCK;
				lock.l_whence=SEEK_SET;
				while(::fcntl(fileno(flock_),F_SETLKW,&lock)!=0 && errno==EINTR)
					;
			}
		}
		
		void rdlock()
		{
			pthread_rwlock_rdlock(plock_);
			if(flock_) {
				struct flock lock;
				memset(&lock,0,sizeof(lock));
				lock.l_type=F_RDLCK;
				lock.l_whence=SEEK_SET;
				while(::fcntl(fileno(flock_),F_SETLKW,&lock)!=0 && errno==EINTR)
					;
			}
		}

		void unlock()
		{
			if(flock_) {
				struct flock lock;
				memset(&lock,0,sizeof(lock));
				lock.l_type=F_UNLCK;
				lock.l_whence=SEEK_SET;
				while(::fcntl(fileno(flock_),F_SETLKW,&lock)!=0 && errno==EINTR)
					;
			}
			pthread_rwlock_unlock(plock_);
		}

		~shared_mutex()
		{
			if(flock_) ::fclose(flock_);
			destroy_rwlock(plock_);
			if(plock_ != &normal_)
				::munmap((char*)plock_,sizeof(pthread_rwlock_t));
		}
	private:
		pthread_rwlock_t *plock_;
		FILE *flock_;
		pthread_rwlock_t normal_;
	};
	
	class shared_mutex::shared_guard : public util::noncopyable {
	public:
		shared_guard(shared_mutex &m) : m_(m) 
		{
			m_.rdlock();
		}
		~shared_guard()
		{
			m_.unlock();
		}
	private:
		shared_mutex &m_;
	};
	
	class shared_mutex::unique_guard : public util::noncopyable {
	public:
		unique_guard(shared_mutex &m) : m_(m) 
		{
			m_.wrlock();
		}
		~unique_guard()
		{
			m_.unlock();
		}
	private:
		shared_mutex &m_;
	};

} // impl
} // cppcms


#endif

